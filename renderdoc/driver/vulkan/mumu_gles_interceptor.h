/******************************************************************************
 * MuMu 12 GLES Compressed Texture Interceptor & Asset Metadata Manifest
 *
 * Intercepts glCompressedTexImage2D / glCompressedTexSubImage2D calls in memory
 * and associates original mobile formats (ASTC/ETC2) with RenderDoc Vulkan Images.
 ******************************************************************************/

#pragma once

#include <stdint.h>
#include <map>
#include "common/common.h"
#include "common/threading.h"
#include "core/core.h"

// GL Compressed Texture Formats
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR 0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR 0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR 0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR 0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR 0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR 0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR 0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR 0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR 0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR 0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR 0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD

#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD

#define GL_COMPRESSED_R11_EAC 0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#define GL_COMPRESSED_RG11_EAC 0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#define GL_COMPRESSED_RGB8_ETC2 0x9274
#define GL_COMPRESSED_SRGB8_ETC2 0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#endif

struct CompressedTexMetadata
{
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t level = 0;
  uint32_t internalformat = 0;
  uint32_t imageSize = 0;
  uint32_t uncompressedSize = 0;
  uint64_t timestamp = 0;
  bool isVulkan = false;
  bool isVideo = false;
  rdcstr videoCodec;
  rdcstr videoPixelFormat;
  float videoFps = 0.0f;
  uint32_t videoFrameCount = 0;

  const char *GetFormatString() const;
  uint32_t GetDecompressedSize() const
  {
    if(uncompressedSize > 0)
      return uncompressedSize;
    return width * height * 4;
  }
  float GetCompressionRatio() const
  {
    uint32_t dec = GetDecompressedSize();
    return dec > 0 ? (float)imageSize / (float)dec : 1.0f;
  }
};

class MuMuGLESInterceptor
{
public:
  static MuMuGLESInterceptor &Inst();

  void Initialise();
  void Shutdown();

  // Called from hooked GLES exports
  void RecordCompressedTex(uint32_t target, int32_t level, uint32_t internalformat,
                           int32_t width, int32_t height, uint32_t imageSize);

  // Called from hooked MuMu libRenderer gfxstream Vulkan translator
  void RecordVulkanCompressedTex(uint32_t vkFormat, uint32_t width, uint32_t height, uint32_t mips);

  // Called from hooked MuMu libRenderer for external video frame buffers (MediaCodec / NV12 / YUV)
  void RecordVideoFrameTex(uint32_t width, uint32_t height, const char *codec,
                           const char *pixelFormat, uint32_t halFormat);

  // Called from WrappedVulkan::vkCreateImage
  bool MatchCompressedTex(uint32_t width, uint32_t height, CompressedTexMetadata &outMeta);

  // Register image mapping
  void RegisterVulkanImage(ResourceId resId, const CompressedTexMetadata &meta);

  // Check if an image is tracked
  bool GetImageMetadata(ResourceId resId, CompressedTexMetadata &outMeta);

  // Export JSON manifest alongside .rdc
  void ExportManifest(const rdcstr &capturePath);

  // Clear per-frame mappings
  void ResetFrame();

private:
  MuMuGLESInterceptor();
  ~MuMuGLESInterceptor();

  Threading::CriticalSection m_Lock;
  rdcarray<CompressedTexMetadata> m_PendingQueue;
  rdcarray<CompressedTexMetadata> m_AllIntercepted;
  std::map<ResourceId, CompressedTexMetadata> m_ActiveManifest;
  std::map<ResourceId, CompressedTexMetadata> m_PersistentRegistry;

  bool m_Hooked = false;
  bool m_RendererHooked = false;
  bool m_InitAttempted = false;
};
