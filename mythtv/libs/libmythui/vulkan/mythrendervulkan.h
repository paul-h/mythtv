#ifndef MYTHRENDERVULKAN_H
#define MYTHRENDERVULKAN_H

// Qt
#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>

// MythTV
#include "mythrender_base.h"

class MythImage;
class MythRenderVulkan;
class MythWindowVulkan;
class MythShaderVulkan;
class MythTextureVulkan;

class MythVulkanObject
{
  public:
    MythVulkanObject(MythRenderVulkan* Render, VkDevice Device, QVulkanDeviceFunctions* Functions);
    bool                    m_valid    { false   };
    MythRenderVulkan*       m_render   { nullptr };
    VkDevice                m_device   { nullptr };
    QVulkanDeviceFunctions* m_devFuncs { nullptr };
};

class MythRenderVulkan : public QObject, public QVulkanWindowRenderer, public MythRender
{
    Q_OBJECT

  public:
    static const std::vector<uint16_t> s_VertexIndices;

    MythRenderVulkan();
   ~MythRenderVulkan() override;

    void SetVulkanWindow           (MythWindowVulkan *VulkanWindow);
    void SetFrameExpected          (void);
    bool GetFrameExpected          (void);
    bool GetFrameStarted           (void);
    bool Init                      (void);
    void EndFrame                  (void);
    QStringList GetDescription(void) override;

    void preInitResources          (void) override;
    void initResources             (void) override;
    void initSwapChainResources    (void) override;
    void releaseSwapChainResources (void) override;
    void releaseResources          (void) override;
    void startNextFrame            (void) override;
    void physicalDeviceLost        (void) override;
    void logicalDeviceLost         (void) override;

    bool            CreateImage(QSize Size, VkFormat Format, VkImageTiling Tiling,
                                VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,
                                VkImage& Image, VkDeviceMemory& ImageMemory);
    void            TransitionImageLayout(VkImage &Image, VkImageLayout OldLayout, VkImageLayout NewLayout);
    void            CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    bool            CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage,
                                 VkMemoryPropertyFlags Properties, VkBuffer& Buffer,
                                 VkDeviceMemory& Memory);
    void            CopyBuffer(VkBuffer Src, VkBuffer Dst, VkDeviceSize Size,
                               VkCommandBuffer CommandBuffer = nullptr);
    VkPipeline      CreatePipeline(MythShaderVulkan* Shader, VkPipelineLayout Layout, const QRect &Viewport);
    VkCommandBuffer CreateSingleUseCommandBuffer(void);
    void            FinishSingleUseCommandBuffer(VkCommandBuffer &Buffer);

  signals:
    void DoFreeResources    (void);
    void PhysicalDeviceLost (void);
    void LogicalDeviceLost  (void);

  private:
    Q_DISABLE_COPY(MythRenderVulkan)

    std::optional<uint32_t> FindMemoryType(uint32_t Filter, VkMemoryPropertyFlags Properties);

    void BeginFrame                (void);
    void DebugVulkan               (void);

    MythWindowVulkan*  m_window        { nullptr };
    VkDevice           m_device        { nullptr };
    QVulkanFunctions*  m_funcs         { nullptr };
    QVulkanDeviceFunctions *m_devFuncs { nullptr };
    bool               m_frameExpected { false   };
    bool               m_frameStarted  { false   };
    QMap<QString,QString> m_debugInfo;
};

#endif
