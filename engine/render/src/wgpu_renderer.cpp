#include <aga/render/renderer.hpp>

#if defined(AGA_BROWSER_WEBGPU)

#include <memory>
#include <utility>
#include <vector>

namespace aga::render {
class Renderer::Impl {
public:
  void initialize(window::Window&, const RendererConfig&) {}

  MeshId create_mesh(const MeshData& mesh) {
    meshes_.push_back(mesh);
    return MeshId{static_cast<std::uint32_t>(meshes_.size())};
  }

  void resize(std::uint32_t, std::uint32_t) {}
  void render(const RenderScene&) {}

private:
  std::vector<MeshData> meshes_;
};

Renderer::Renderer() : impl_(std::make_unique<Impl>()) {}
Renderer::~Renderer() = default;
Renderer::Renderer(Renderer&&) noexcept = default;
Renderer& Renderer::operator=(Renderer&&) noexcept = default;

void Renderer::initialize(window::Window& window, const RendererConfig& config) {
  impl_->initialize(window, config);
}

MeshId Renderer::create_mesh(const MeshData& mesh) { return impl_->create_mesh(mesh); }

void Renderer::resize(std::uint32_t width, std::uint32_t height) { impl_->resize(width, height); }

void Renderer::render(const RenderScene& scene) { impl_->render(scene); }
} // namespace aga::render

#else

#include <aga/window/native_window.hpp>
#include <aga/window/window.hpp>

#include <webgpu/webgpu.h>

#if defined(GLFW_EXPOSE_NATIVE_X11)
#include <GLFW/glfw3native.h>
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace aga::render {
namespace {
constexpr WGPUTextureFormat depth_format = WGPUTextureFormat_Depth24Plus;

constexpr std::string_view mesh_shader = R"wgsl(
struct DrawUniforms {
  view_projection: mat4x4<f32>,
  model: mat4x4<f32>,
  color: vec4<f32>,
};

@group(0) @binding(0)
var<uniform> uniforms: DrawUniforms;

struct VertexInput {
  @location(0) position: vec3<f32>,
  @location(1) normal: vec3<f32>,
};

struct VertexOutput {
  @builtin(position) clip_position: vec4<f32>,
  @location(0) normal: vec3<f32>,
  @location(1) color: vec4<f32>,
};

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
  var output: VertexOutput;
  output.clip_position = uniforms.view_projection * uniforms.model * vec4<f32>(input.position, 1.0);
  output.normal = normalize((uniforms.model * vec4<f32>(input.normal, 0.0)).xyz);
  output.color = uniforms.color;
  return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
  let light_dir = normalize(vec3<f32>(0.4, 0.7, 0.6));
  let light = max(dot(normalize(input.normal), light_dir), 0.18);
  return vec4<f32>(input.color.rgb * light, input.color.a);
}
)wgsl";

WGPUStringView wgpu_string(std::string_view value) {
  return WGPUStringView{value.data(), value.size()};
}

void check(bool condition, std::string_view message) {
  if (!condition) {
    throw std::runtime_error(std::string(message));
  }
}

struct AdapterRequest {
  WGPUAdapter adapter = nullptr;
  WGPURequestAdapterStatus status = WGPURequestAdapterStatus_Error;
};

struct DeviceRequest {
  WGPUDevice device = nullptr;
  WGPURequestDeviceStatus status = WGPURequestDeviceStatus_Error;
};

struct GpuMesh {
  WGPUBuffer vertex_buffer = nullptr;
  WGPUBuffer index_buffer = nullptr;
  std::uint32_t index_count = 0;
};

struct PreparedDraw {
  const GpuMesh* mesh = nullptr;
  WGPUBuffer uniform_buffer = nullptr;
  WGPUBindGroup bind_group = nullptr;
};

struct alignas(16) DrawUniforms {
  glm::mat4 view_projection{1.0F};
  glm::mat4 model{1.0F};
  glm::vec4 color{1.0F};
};

void wait_for(WGPUInstance instance, WGPUFuture future) {
  auto wait_info = WGPU_FUTURE_WAIT_INFO_INIT;
  wait_info.future = future;
  wgpuInstanceWaitAny(instance, 1, &wait_info, std::numeric_limits<std::uint64_t>::max());
}

WGPUAdapter request_adapter(WGPUInstance instance, WGPUSurface surface) {
  AdapterRequest request;
  auto options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
  options.compatibleSurface = surface;
  options.powerPreference = WGPUPowerPreference_HighPerformance;

  auto callback = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
  callback.mode = WGPUCallbackMode_WaitAnyOnly;
  callback.userdata1 = &request;
  callback.callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView,
                         void* userdata1, void*) {
    auto* result = static_cast<AdapterRequest*>(userdata1);
    result->status = status;
    result->adapter = adapter;
  };

  wait_for(instance, wgpuInstanceRequestAdapter(instance, &options, callback));
  check(request.status == WGPURequestAdapterStatus_Success && request.adapter != nullptr,
        "failed to request webgpu adapter");
  return request.adapter;
}

WGPUDevice request_device(WGPUInstance instance, WGPUAdapter adapter) {
  DeviceRequest request;
  auto descriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
  descriptor.label = wgpu_string("aga device");

  auto callback = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
  callback.mode = WGPUCallbackMode_WaitAnyOnly;
  callback.userdata1 = &request;
  callback.callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView,
                         void* userdata1, void*) {
    auto* result = static_cast<DeviceRequest*>(userdata1);
    result->status = status;
    result->device = device;
  };

  wait_for(instance, wgpuAdapterRequestDevice(adapter, &descriptor, callback));
  check(request.status == WGPURequestDeviceStatus_Success && request.device != nullptr,
        "failed to request webgpu device");
  return request.device;
}

WGPUSurface create_surface(WGPUInstance instance, window::Window& window) {
#if defined(GLFW_EXPOSE_NATIVE_X11)
  auto xlib_source = WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT;
  xlib_source.display = glfwGetX11Display();
  xlib_source.window =
      static_cast<std::uint64_t>(glfwGetX11Window(window::detail::glfw_handle(window)));

  auto descriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
  descriptor.label = wgpu_string("aga surface");
  descriptor.nextInChain = &xlib_source.chain;
  return wgpuInstanceCreateSurface(instance, &descriptor);
#else
  (void)instance;
  (void)window;
  throw std::runtime_error("this platform does not have an aga webgpu surface backend");
#endif
}

WGPUTextureFormat choose_surface_format(WGPUSurface surface, WGPUAdapter adapter) {
  auto capabilities = WGPU_SURFACE_CAPABILITIES_INIT;
  check(wgpuSurfaceGetCapabilities(surface, adapter, &capabilities) == WGPUStatus_Success,
        "failed to read surface capabilities");

  WGPUTextureFormat selected =
      capabilities.formatCount > 0 ? capabilities.formats[0] : WGPUTextureFormat_BGRA8Unorm;
  for (std::size_t i = 0; i < capabilities.formatCount; ++i) {
    const auto format = capabilities.formats[i];
    if (format == WGPUTextureFormat_BGRA8UnormSrgb || format == WGPUTextureFormat_RGBA8UnormSrgb) {
      selected = format;
      break;
    }
  }

  wgpuSurfaceCapabilitiesFreeMembers(capabilities);
  return selected;
}
} // namespace

class Renderer::Impl {
public:
  ~Impl() {
    for (const auto& draw : prepared_draws_) {
      release_prepared_draw(draw);
    }
    for (auto& mesh : meshes_) {
      if (mesh.vertex_buffer != nullptr) {
        wgpuBufferRelease(mesh.vertex_buffer);
      }
      if (mesh.index_buffer != nullptr) {
        wgpuBufferRelease(mesh.index_buffer);
      }
    }
    release_depth();
    if (pipeline_ != nullptr) {
      wgpuRenderPipelineRelease(pipeline_);
    }
    if (pipeline_layout_ != nullptr) {
      wgpuPipelineLayoutRelease(pipeline_layout_);
    }
    if (draw_bind_group_layout_ != nullptr) {
      wgpuBindGroupLayoutRelease(draw_bind_group_layout_);
    }
    if (queue_ != nullptr) {
      wgpuQueueRelease(queue_);
    }
    if (device_ != nullptr) {
      wgpuDeviceRelease(device_);
    }
    if (adapter_ != nullptr) {
      wgpuAdapterRelease(adapter_);
    }
    if (surface_ != nullptr) {
      wgpuSurfaceRelease(surface_);
    }
    if (instance_ != nullptr) {
      wgpuInstanceRelease(instance_);
    }
  }

  void initialize(window::Window& window, const RendererConfig& config) {
    config_ = config;

    auto descriptor = WGPU_INSTANCE_DESCRIPTOR_INIT;
    instance_ = wgpuCreateInstance(&descriptor);
    check(instance_ != nullptr, "failed to create webgpu instance");

    surface_ = create_surface(instance_, window);
    check(surface_ != nullptr, "failed to create webgpu surface");

    adapter_ = request_adapter(instance_, surface_);
    device_ = request_device(instance_, adapter_);
    queue_ = wgpuDeviceGetQueue(device_);
    surface_format_ = choose_surface_format(surface_, adapter_);

    create_pipeline();

    const auto [width, height] = window.framebuffer_size();
    resize(static_cast<std::uint32_t>(std::max(width, 1)),
           static_cast<std::uint32_t>(std::max(height, 1)));
  }

  MeshId create_mesh(const MeshData& mesh) {
    check(!mesh.vertices.empty(), "mesh has no vertices");
    check(!mesh.indices.empty(), "mesh has no indices");

    GpuMesh gpu_mesh;
    gpu_mesh.vertex_buffer =
        create_buffer(mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex),
                      WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst, "aga vertex buffer");
    gpu_mesh.index_buffer =
        create_buffer(mesh.indices.data(), mesh.indices.size() * sizeof(std::uint32_t),
                      WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst, "aga index buffer");
    gpu_mesh.index_count = static_cast<std::uint32_t>(mesh.indices.size());

    meshes_.push_back(gpu_mesh);
    return MeshId{static_cast<std::uint32_t>(meshes_.size())};
  }

  void resize(std::uint32_t width, std::uint32_t height) {
    width = std::max(width, 1U);
    height = std::max(height, 1U);
    if (width == width_ && height == height_) {
      return;
    }

    width_ = width;
    height_ = height;

    auto config = WGPU_SURFACE_CONFIGURATION_INIT;
    config.device = device_;
    config.format = surface_format_;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.width = width_;
    config.height = height_;
    config.presentMode = config_.vsync ? WGPUPresentMode_Fifo : WGPUPresentMode_Immediate;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;
    wgpuSurfaceConfigure(surface_, &config);

    create_depth_resources();
  }

  void render(const RenderScene& scene) {
    if (width_ == 0 || height_ == 0) {
      return;
    }

    prepare_draws(scene);

    auto surface_texture = WGPU_SURFACE_TEXTURE_INIT;
    wgpuSurfaceGetCurrentTexture(surface_, &surface_texture);
    if (surface_texture.status == WGPUSurfaceGetCurrentTextureStatus_Outdated ||
        surface_texture.status == WGPUSurfaceGetCurrentTextureStatus_Lost) {
      resize(width_, height_);
      release_prepared_draws();
      return;
    }

    if (surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
      release_prepared_draws();
      return;
    }

    WGPUTextureView backbuffer = wgpuTextureCreateView(surface_texture.texture, nullptr);
    auto encoder_desc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
    encoder_desc.label = wgpu_string("aga frame encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device_, &encoder_desc);

    auto color_attachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;
    color_attachment.view = backbuffer;
    color_attachment.loadOp = WGPULoadOp_Clear;
    color_attachment.storeOp = WGPUStoreOp_Store;
    color_attachment.clearValue = {config_.clear_color.r, config_.clear_color.g,
                                   config_.clear_color.b, config_.clear_color.a};

    auto depth_attachment = WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;
    depth_attachment.view = depth_view_;
    depth_attachment.depthLoadOp = WGPULoadOp_Clear;
    depth_attachment.depthStoreOp = WGPUStoreOp_Store;
    depth_attachment.depthClearValue = 1.0F;

    auto pass_desc = WGPU_RENDER_PASS_DESCRIPTOR_INIT;
    pass_desc.label = wgpu_string("aga mesh pass");
    pass_desc.colorAttachmentCount = 1;
    pass_desc.colorAttachments = &color_attachment;
    pass_desc.depthStencilAttachment = &depth_attachment;

    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &pass_desc);
    wgpuRenderPassEncoderSetPipeline(pass, pipeline_);

    for (const auto& draw : prepared_draws_) {
      wgpuRenderPassEncoderSetBindGroup(pass, 0, draw.bind_group, 0, nullptr);
      wgpuRenderPassEncoderSetVertexBuffer(pass, 0, draw.mesh->vertex_buffer, 0, WGPU_WHOLE_SIZE);
      wgpuRenderPassEncoderSetIndexBuffer(pass, draw.mesh->index_buffer, WGPUIndexFormat_Uint32, 0,
                                          WGPU_WHOLE_SIZE);
      wgpuRenderPassEncoderDrawIndexed(pass, draw.mesh->index_count, 1, 0, 0, 0);
    }

    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);

    auto command_desc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
    command_desc.label = wgpu_string("aga frame commands");
    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, &command_desc);
    wgpuQueueSubmit(queue_, 1, &commands);
    wgpuSurfacePresent(surface_);

    wgpuCommandBufferRelease(commands);
    wgpuCommandEncoderRelease(encoder);
    wgpuTextureViewRelease(backbuffer);
    wgpuTextureRelease(surface_texture.texture);
    wgpuInstanceProcessEvents(instance_);
    release_prepared_draws();
  }

private:
  WGPUBuffer create_buffer(const void* data, std::size_t size, WGPUBufferUsage usage,
                           std::string_view label) {
    auto descriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
    descriptor.label = wgpu_string(label);
    descriptor.size = size;
    descriptor.usage = usage;
    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device_, &descriptor);
    check(buffer != nullptr, "failed to create webgpu buffer");
    if (data != nullptr && size > 0) {
      wgpuQueueWriteBuffer(queue_, buffer, 0, data, size);
    }
    return buffer;
  }

  WGPUBindGroup create_draw_bind_group(WGPUBuffer uniform_buffer) {
    auto bind_entry = WGPU_BIND_GROUP_ENTRY_INIT;
    bind_entry.binding = 0;
    bind_entry.buffer = uniform_buffer;
    bind_entry.offset = 0;
    bind_entry.size = sizeof(DrawUniforms);

    auto bind_desc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    bind_desc.label = wgpu_string("aga draw bind group");
    bind_desc.layout = draw_bind_group_layout_;
    bind_desc.entryCount = 1;
    bind_desc.entries = &bind_entry;
    return wgpuDeviceCreateBindGroup(device_, &bind_desc);
  }

  void prepare_draws(const RenderScene& scene) {
    release_prepared_draws();
    prepared_draws_.reserve(scene.items.size());

    for (const auto& item : scene.items) {
      if (item.mesh.value == 0 || item.mesh.value > meshes_.size()) {
        continue;
      }

      DrawUniforms uniforms;
      uniforms.view_projection = scene.view_projection;
      uniforms.model = item.model;
      uniforms.color = item.color;

      WGPUBuffer uniform_buffer =
          create_buffer(&uniforms, sizeof(uniforms),
                        WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst, "aga draw uniforms");
      WGPUBindGroup bind_group = create_draw_bind_group(uniform_buffer);
      prepared_draws_.push_back(
          PreparedDraw{&meshes_[item.mesh.value - 1], uniform_buffer, bind_group});
    }
  }

  void create_pipeline() {
    auto shader_source = WGPU_SHADER_SOURCE_WGSL_INIT;
    shader_source.code = wgpu_string(mesh_shader);
    auto shader_desc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    shader_desc.label = wgpu_string("aga mesh shader");
    shader_desc.nextInChain = &shader_source.chain;
    WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device_, &shader_desc);
    check(shader != nullptr, "failed to create mesh shader module");

    auto layout_entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    layout_entry.binding = 0;
    layout_entry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    layout_entry.buffer.type = WGPUBufferBindingType_Uniform;
    layout_entry.buffer.minBindingSize = sizeof(DrawUniforms);

    auto bgl_desc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    bgl_desc.label = wgpu_string("aga draw bind group layout");
    bgl_desc.entryCount = 1;
    bgl_desc.entries = &layout_entry;
    draw_bind_group_layout_ = wgpuDeviceCreateBindGroupLayout(device_, &bgl_desc);

    auto pipeline_layout_desc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    pipeline_layout_desc.label = wgpu_string("aga pipeline layout");
    pipeline_layout_desc.bindGroupLayoutCount = 1;
    pipeline_layout_desc.bindGroupLayouts = &draw_bind_group_layout_;
    pipeline_layout_ = wgpuDeviceCreatePipelineLayout(device_, &pipeline_layout_desc);

    WGPUVertexAttribute attributes[2] = {};
    attributes[0] = WGPU_VERTEX_ATTRIBUTE_INIT;
    attributes[0].format = WGPUVertexFormat_Float32x3;
    attributes[0].offset = offsetof(Vertex, position);
    attributes[0].shaderLocation = 0;
    attributes[1] = WGPU_VERTEX_ATTRIBUTE_INIT;
    attributes[1].format = WGPUVertexFormat_Float32x3;
    attributes[1].offset = offsetof(Vertex, normal);
    attributes[1].shaderLocation = 1;

    auto vertex_layout = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
    vertex_layout.arrayStride = sizeof(Vertex);
    vertex_layout.stepMode = WGPUVertexStepMode_Vertex;
    vertex_layout.attributeCount = 2;
    vertex_layout.attributes = attributes;

    auto color_target = WGPU_COLOR_TARGET_STATE_INIT;
    color_target.format = surface_format_;
    color_target.writeMask = WGPUColorWriteMask_All;

    auto fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = shader;
    fragment.entryPoint = wgpu_string("fs_main");
    fragment.targetCount = 1;
    fragment.targets = &color_target;

    auto depth = WGPU_DEPTH_STENCIL_STATE_INIT;
    depth.format = depth_format;
    depth.depthWriteEnabled = WGPUOptionalBool_True;
    depth.depthCompare = WGPUCompareFunction_Less;

    auto pipeline_desc = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    pipeline_desc.label = wgpu_string("aga mesh pipeline");
    pipeline_desc.layout = pipeline_layout_;
    pipeline_desc.vertex.module = shader;
    pipeline_desc.vertex.entryPoint = wgpu_string("vs_main");
    pipeline_desc.vertex.bufferCount = 1;
    pipeline_desc.vertex.buffers = &vertex_layout;
    pipeline_desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline_desc.primitive.frontFace = WGPUFrontFace_CCW;
    pipeline_desc.primitive.cullMode = WGPUCullMode_Back;
    pipeline_desc.depthStencil = &depth;
    pipeline_desc.fragment = &fragment;
    pipeline_desc.multisample.count = 1;

    pipeline_ = wgpuDeviceCreateRenderPipeline(device_, &pipeline_desc);
    wgpuShaderModuleRelease(shader);
    check(pipeline_ != nullptr, "failed to create webgpu render pipeline");
  }

  void create_depth_resources() {
    release_depth();

    auto texture_desc = WGPU_TEXTURE_DESCRIPTOR_INIT;
    texture_desc.label = wgpu_string("aga depth texture");
    texture_desc.usage = WGPUTextureUsage_RenderAttachment;
    texture_desc.dimension = WGPUTextureDimension_2D;
    texture_desc.size = WGPUExtent3D{width_, height_, 1};
    texture_desc.format = depth_format;
    texture_desc.mipLevelCount = 1;
    texture_desc.sampleCount = 1;

    depth_texture_ = wgpuDeviceCreateTexture(device_, &texture_desc);
    depth_view_ = wgpuTextureCreateView(depth_texture_, nullptr);
  }

  void release_depth() {
    if (depth_view_ != nullptr) {
      wgpuTextureViewRelease(depth_view_);
      depth_view_ = nullptr;
    }
    if (depth_texture_ != nullptr) {
      wgpuTextureRelease(depth_texture_);
      depth_texture_ = nullptr;
    }
  }

  void release_prepared_draw(PreparedDraw draw) {
    if (draw.bind_group != nullptr) {
      wgpuBindGroupRelease(draw.bind_group);
    }
    if (draw.uniform_buffer != nullptr) {
      wgpuBufferRelease(draw.uniform_buffer);
    }
  }

  void release_prepared_draws() {
    for (const auto& draw : prepared_draws_) {
      release_prepared_draw(draw);
    }
    prepared_draws_.clear();
  }

  RendererConfig config_{};
  WGPUInstance instance_ = nullptr;
  WGPUSurface surface_ = nullptr;
  WGPUAdapter adapter_ = nullptr;
  WGPUDevice device_ = nullptr;
  WGPUQueue queue_ = nullptr;
  WGPUTextureFormat surface_format_ = WGPUTextureFormat_BGRA8Unorm;
  WGPUBindGroupLayout draw_bind_group_layout_ = nullptr;
  WGPUPipelineLayout pipeline_layout_ = nullptr;
  WGPURenderPipeline pipeline_ = nullptr;
  WGPUTexture depth_texture_ = nullptr;
  WGPUTextureView depth_view_ = nullptr;
  std::uint32_t width_ = 0;
  std::uint32_t height_ = 0;
  std::vector<GpuMesh> meshes_;
  std::vector<PreparedDraw> prepared_draws_;
};

Renderer::Renderer() : impl_(std::make_unique<Impl>()) {}
Renderer::~Renderer() = default;
Renderer::Renderer(Renderer&&) noexcept = default;
Renderer& Renderer::operator=(Renderer&&) noexcept = default;

void Renderer::initialize(window::Window& window, const RendererConfig& config) {
  impl_->initialize(window, config);
}

MeshId Renderer::create_mesh(const MeshData& mesh) { return impl_->create_mesh(mesh); }

void Renderer::resize(std::uint32_t width, std::uint32_t height) { impl_->resize(width, height); }

void Renderer::render(const RenderScene& scene) { impl_->render(scene); }
} // namespace aga::render

#endif
