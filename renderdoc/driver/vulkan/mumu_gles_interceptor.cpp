/******************************************************************************
 * MuMu 12 GLES & Vulkan Compressed Texture Interceptor & Asset Metadata Manifest
 *
 * Intercepts MuMu libRenderer gfxstream Vulkan format conversion (ASTC/ETC2 -> RGBA8)
 * and glCompressedTexImage2D calls, associating original mobile formats with
 * RenderDoc Vulkan Images.
 ******************************************************************************/

#include "mumu_gles_interceptor.h"
#include <windows.h>
#include <tlhelp32.h>
#include "common/common.h"
#include "driver/vulkan/vk_core.h"
#include "driver/vulkan/vk_resources.h"
#include "hooks/hooks.h"
#include "os/os_specific.h"

// Format name stringification
const char *CompressedTexMetadata::GetFormatString() const
{
  if(isVideo)
  {
    if(!videoPixelFormat.empty())
      return videoPixelFormat.c_str();
    return "VIDEO_FRAME";
  }

  if(isVulkan || (internalformat >= 147 && internalformat <= 184))
  {
    switch(internalformat)
    {
      case 157: return "ASTC_4x4_UNORM";
      case 158: return "ASTC_4x4_SRGB";
      case 159: return "ASTC_5x4_UNORM";
      case 160: return "ASTC_5x4_SRGB";
      case 161: return "ASTC_5x5_UNORM";
      case 162: return "ASTC_5x5_SRGB";
      case 163: return "ASTC_6x5_UNORM";
      case 164: return "ASTC_6x5_SRGB";
      case 165: return "ASTC_6x6_UNORM";
      case 166: return "ASTC_6x6_SRGB";
      case 167: return "ASTC_8x5_UNORM";
      case 168: return "ASTC_8x5_SRGB";
      case 169: return "ASTC_8x6_UNORM";
      case 170: return "ASTC_8x6_SRGB";
      case 171: return "ASTC_8x8_UNORM";
      case 172: return "ASTC_8x8_SRGB";
      case 173: return "ASTC_10x5_UNORM";
      case 174: return "ASTC_10x5_SRGB";
      case 175: return "ASTC_10x6_UNORM";
      case 176: return "ASTC_10x6_SRGB";
      case 177: return "ASTC_10x8_UNORM";
      case 178: return "ASTC_10x8_SRGB";
      case 179: return "ASTC_10x10_UNORM";
      case 180: return "ASTC_10x10_SRGB";
      case 181: return "ASTC_12x10_UNORM";
      case 182: return "ASTC_12x10_SRGB";
      case 183: return "ASTC_12x12_UNORM";
      case 184: return "ASTC_12x12_SRGB";

      case 147: return "ETC2_RGB8";
      case 148: return "ETC2_SRGB8";
      case 149: return "ETC2_RGB8A1";
      case 150: return "ETC2_SRGB8A1";
      case 151: return "ETC2_RGBA8";
      case 152: return "ETC2_SRGB8A8";
      case 153: return "EAC_R11";
      case 154: return "EAC_SIGNED_R11";
      case 155: return "EAC_RG11";
      case 156: return "EAC_SIGNED_RG11";
      default: break;
    }
  }

  switch(internalformat)
  {
    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: return "ASTC_4x4_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: return "ASTC_5x4_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: return "ASTC_5x5_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: return "ASTC_6x5_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: return "ASTC_6x6_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: return "ASTC_8x5_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: return "ASTC_8x6_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: return "ASTC_8x8_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: return "ASTC_10x5_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: return "ASTC_10x6_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: return "ASTC_10x8_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: return "ASTC_10x10_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: return "ASTC_12x10_RGBA";
    case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: return "ASTC_12x12_RGBA";

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: return "ASTC_4x4_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: return "ASTC_5x4_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: return "ASTC_5x5_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: return "ASTC_6x5_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: return "ASTC_6x6_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: return "ASTC_8x5_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: return "ASTC_8x6_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: return "ASTC_8x8_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: return "ASTC_10x5_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: return "ASTC_10x6_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: return "ASTC_10x8_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: return "ASTC_10x10_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: return "ASTC_12x10_SRGB8_A8";
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: return "ASTC_12x12_SRGB8_A8";

    case GL_COMPRESSED_RGB8_ETC2: return "ETC2_RGB8";
    case GL_COMPRESSED_SRGB8_ETC2: return "ETC2_SRGB8";
    case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: return "ETC2_RGB8_A1";
    case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return "ETC2_SRGB8_A1";
    case GL_COMPRESSED_RGBA8_ETC2_EAC: return "ETC2_RGBA8_EAC";
    case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: return "ETC2_SRGB8_A8_EAC";
    case GL_COMPRESSED_R11_EAC: return "EAC_R11";
    case GL_COMPRESSED_SIGNED_R11_EAC: return "EAC_SIGNED_R11";
    case GL_COMPRESSED_RG11_EAC: return "EAC_RG11";
    case GL_COMPRESSED_SIGNED_RG11_EAC: return "EAC_SIGNED_RG11";
    default: break;
  }
  return "UNKNOWN_COMPRESSED";
}

static void GetVulkanBlockInfo(uint32_t vkFormat, uint32_t &blockW, uint32_t &blockH,
                               uint32_t &bytesPerBlock)
{
  switch(vkFormat)
  {
    // ASTC: all 16 bytes per block
    case 157: case 158: blockW = 4; blockH = 4; bytesPerBlock = 16; break;
    case 159: case 160: blockW = 5; blockH = 4; bytesPerBlock = 16; break;
    case 161: case 162: blockW = 5; blockH = 5; bytesPerBlock = 16; break;
    case 163: case 164: blockW = 6; blockH = 5; bytesPerBlock = 16; break;
    case 165: case 166: blockW = 6; blockH = 6; bytesPerBlock = 16; break;
    case 167: case 168: blockW = 8; blockH = 5; bytesPerBlock = 16; break;
    case 169: case 170: blockW = 8; blockH = 6; bytesPerBlock = 16; break;
    case 171: case 172: blockW = 8; blockH = 8; bytesPerBlock = 16; break;
    case 173: case 174: blockW = 10; blockH = 5; bytesPerBlock = 16; break;
    case 175: case 176: blockW = 10; blockH = 6; bytesPerBlock = 16; break;
    case 177: case 178: blockW = 10; blockH = 8; bytesPerBlock = 16; break;
    case 179: case 180: blockW = 10; blockH = 10; bytesPerBlock = 16; break;
    case 181: case 182: blockW = 12; blockH = 10; bytesPerBlock = 16; break;
    case 183: case 184: blockW = 12; blockH = 12; bytesPerBlock = 16; break;

    // ETC2 / EAC
    case 147: case 148: // ETC2 RGB8
    case 149: case 150: // ETC2 RGB8A1
    case 153: case 154: // EAC R11
      blockW = 4; blockH = 4; bytesPerBlock = 8; break;

    case 151: case 152: // ETC2 RGBA8
    case 155: case 156: // EAC RG11
      blockW = 4; blockH = 4; bytesPerBlock = 16; break;

    default:
      blockW = 1; blockH = 1; bytesPerBlock = 4; break;
  }
}

static uint32_t CalcVulkanCompressedBytes(uint32_t vkFormat, uint32_t w, uint32_t h, uint32_t mips)
{
  uint32_t bw = 1, bh = 1, bpb = 4;
  GetVulkanBlockInfo(vkFormat, bw, bh, bpb);
  if(mips == 0) mips = 1;

  uint32_t total = 0;
  for(uint32_t m = 0; m < mips; m++)
  {
    uint32_t mw = RDCMAX(1U, w >> m);
    uint32_t mh = RDCMAX(1U, h >> m);
    uint32_t bx = (mw + bw - 1) / bw;
    uint32_t by = (mh + bh - 1) / bh;
    total += bx * by * bpb;
  }
  return total;
}

static uint32_t CalcVulkanUncompressedBytes(uint32_t w, uint32_t h, uint32_t mips)
{
  if(mips == 0) mips = 1;
  uint32_t total = 0;
  for(uint32_t m = 0; m < mips; m++)
  {
    uint32_t mw = RDCMAX(1U, w >> m);
    uint32_t mh = RDCMAX(1U, h >> m);
    total += mw * mh * 4;
  }
  return total;
}

// Write a line to diagnostic log file
static void DiagLog(const char *msg)
{
  FILE *f = NULL;
  fopen_s(&f, "E:/Task/RDC_Res/mumu_hook_diag.log", "a");
  if(f)
  {
    fputs(msg, f);
    fputs("\n", f);
    fclose(f);
  }
}

// 16-byte hook (for standard API stubs)
static void *InstallRobustHook(void *targetFunc, void *hookFunc)
{
  if(!targetFunc || !hookFunc)
    return NULL;

  uint8_t *code = (uint8_t *)targetFunc;
  uint8_t *realTarget = code;

  if(code[0] == 0xE9)
  {
    int32_t rel = *(int32_t *)(code + 1);
    realTarget = code + 5 + rel;
    RDCLOG("[MuMuGLESInterceptor] Target %p is an E9 stub jumping to %p", targetFunc, realTarget);
  }

  uint8_t *trampoline =
      (uint8_t *)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if(!trampoline)
  {
    RDCERR("[MuMuGLESInterceptor] VirtualAlloc failed for trampoline!");
    return realTarget;
  }

  memcpy(trampoline, realTarget, 16);

  uint8_t *jumpBack = trampoline + 16;
  jumpBack[0] = 0xFF;
  jumpBack[1] = 0x25;
  jumpBack[2] = 0x00;
  jumpBack[3] = 0x00;
  jumpBack[4] = 0x00;
  jumpBack[5] = 0x00;
  *(uint64_t *)(jumpBack + 6) = (uint64_t)(realTarget + 16);

  FlushInstructionCache(GetCurrentProcess(), trampoline, 32);

  uint8_t patch[16];
  memset(patch, 0xCC, 16);
  patch[0] = 0xFF;
  patch[1] = 0x25;
  patch[2] = 0x00;
  patch[3] = 0x00;
  patch[4] = 0x00;
  patch[5] = 0x00;
  *(uint64_t *)(patch + 6) = (uint64_t)hookFunc;

  DWORD oldProtect = 0;
  if(VirtualProtect(realTarget, 16, PAGE_EXECUTE_READWRITE, &oldProtect))
  {
    memcpy(realTarget, patch, 16);
    VirtualProtect(realTarget, 16, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), realTarget, 16);
    RDCLOG("[MuMuGLESInterceptor] In-memory hook installed at realTarget %p -> %p (trampoline %p)",
           realTarget, hookFunc, trampoline);
  }
  else
  {
    RDCERR("[MuMuGLESInterceptor] VirtualProtect failed on realTarget %p (err %u)", realTarget,
           GetLastError());
  }

  if(targetFunc != realTarget)
  {
    if(VirtualProtect(targetFunc, 16, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
      memcpy(targetFunc, patch, 16);
      VirtualProtect(targetFunc, 16, oldProtect, &oldProtect);
      FlushInstructionCache(GetCurrentProcess(), targetFunc, 16);
      RDCLOG("[MuMuGLESInterceptor] In-memory hook also patched export stub %p -> %p", targetFunc,
             hookFunc);
    }
  }

  return trampoline;
}

// 19-byte instruction-aligned hook for transformImpl_VkImageCreateInfo_tohost
static void *InstallRobustHook19(void *targetFunc, void *hookFunc)
{
  if(!targetFunc || !hookFunc)
    return NULL;

  uint8_t *realTarget = (uint8_t *)targetFunc;

  uint8_t *trampoline =
      (uint8_t *)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if(!trampoline)
  {
    RDCERR("[MuMuGLESInterceptor] VirtualAlloc failed for 19-byte trampoline!");
    return realTarget;
  }

  // Copy exactly 19 bytes (instruction-aligned boundary: 8 pushes + sub rsp, 0x1d8)
  memcpy(trampoline, realTarget, 19);

  // Jump back to realTarget + 19
  uint8_t *jumpBack = trampoline + 19;
  jumpBack[0] = 0xFF;
  jumpBack[1] = 0x25;
  jumpBack[2] = 0x00;
  jumpBack[3] = 0x00;
  jumpBack[4] = 0x00;
  jumpBack[5] = 0x00;
  *(uint64_t *)(jumpBack + 6) = (uint64_t)(realTarget + 19);

  FlushInstructionCache(GetCurrentProcess(), trampoline, 40);

  // Patch realTarget with 14-byte jump + 5 NOPs = 19 bytes
  uint8_t patch[19];
  patch[0] = 0xFF;
  patch[1] = 0x25;
  patch[2] = 0x00;
  patch[3] = 0x00;
  patch[4] = 0x00;
  patch[5] = 0x00;
  *(uint64_t *)(patch + 6) = (uint64_t)hookFunc;
  patch[14] = 0x90;
  patch[15] = 0x90;
  patch[16] = 0x90;
  patch[17] = 0x90;
  patch[18] = 0x90;

  DWORD oldProtect = 0;
  if(VirtualProtect(realTarget, 19, PAGE_EXECUTE_READWRITE, &oldProtect))
  {
    memcpy(realTarget, patch, 19);
    VirtualProtect(realTarget, 19, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), realTarget, 19);
    RDCLOG("[MuMuGLESInterceptor] In-memory 19-byte hook installed at %p -> %p (trampoline %p)",
           realTarget, hookFunc, trampoline);
  }
  else
  {
    RDCERR("[MuMuGLESInterceptor] VirtualProtect failed on 19-byte target %p (err %u)", realTarget,
           GetLastError());
  }

  return trampoline;
}

// libRenderer gfxstream Vulkan hook
typedef void (*PFN_transform_VkImageCreateInfo)(void *thisPtr, VkImageCreateInfo *pCreateInfos,
                                                uint32_t count, void *arg4);
static PFN_transform_VkImageCreateInfo s_Orig_transform_VkImageCreateInfo = NULL;

static void Hooked_transform_VkImageCreateInfo(void *thisPtr, VkImageCreateInfo *pCreateInfos,
                                               uint32_t count, void *arg4)
{
  if(pCreateInfos && count > 0)
  {
    for(uint32_t i = 0; i < count; i++)
    {
      uint32_t fmt = (uint32_t)pCreateInfos[i].format;
      uint32_t w = pCreateInfos[i].extent.width;
      uint32_t h = pCreateInfos[i].extent.height;
      uint32_t mips = pCreateInfos[i].mipLevels;

      // 1. ASTC: 157..184, ETC2: 147..156
      if(fmt >= 147 && fmt <= 184)
      {
        MuMuGLESInterceptor::Inst().RecordVulkanCompressedTex(fmt, w, h, mips);
        continue;
      }

      // 2. Vulkan YCbCr native formats: 1000156000 to 1000156030
      if(fmt >= 1000156000 && fmt <= 1000156030)
      {
        const char *pixelFmt = "NV12 (YUV420)";
        if(fmt == 1000156000)
          pixelFmt = "YUV420P";
        else if(fmt == 1000156003)
          pixelFmt = "P010_10BIT_HDR";

        MuMuGLESInterceptor::Inst().RecordVideoFrameTex(w, h, "Vulkan_YCbCr", pixelFmt, fmt);
        continue;
      }

      // 3. Android Gralloc native buffer in pNext chain (MediaCodec / AHardwareBuffer)
      const void *currNext = pCreateInfos[i].pNext;
      while(currNext)
      {
        const uint32_t *pStruct = (const uint32_t *)currNext;
        uint32_t sType = 0;
        const void *nextPtr = NULL;

        __try
        {
          sType = pStruct[0];
          nextPtr = *(const void **)((const uint8_t *)currNext + 8);

          if(sType == 1000010000) // VK_STRUCTURE_TYPE_NATIVE_BUFFER_ANDROID
          {
            const void *handle = *(const void **)((const uint8_t *)currNext + 16);
            if(handle)
            {
              int halFmt = *(const int *)((const uint8_t *)handle + 68);
              if(halFmt == 0x23 || halFmt == 0x22 || halFmt == 0x32315659 || halFmt == 0x11)
              {
                const char *pixelFmt = "NV12 (YUV420)";
                if(halFmt == 0x32315659)
                  pixelFmt = "YV12";
                else if(halFmt == 0x11)
                  pixelFmt = "NV21";
                else if(halFmt == 0x22)
                  pixelFmt = "NV12 (MediaCodec)";

                MuMuGLESInterceptor::Inst().RecordVideoFrameTex(w, h, "MediaCodec_Video", pixelFmt,
                                                               (uint32_t)halFmt);
                break;
              }
            }
          }
          else if(sType == 1000156002) // VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID
          {
            uint64_t extFormat = *(const uint64_t *)((const uint8_t *)currNext + 16);
            if(extFormat != 0)
            {
              MuMuGLESInterceptor::Inst().RecordVideoFrameTex(w, h, "Android_External",
                                                             "External_YUV", (uint32_t)extFormat);
              break;
            }
          }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
          break;
        }

        currNext = nextPtr;
      }
    }
  }

  if(s_Orig_transform_VkImageCreateInfo)
  {
    s_Orig_transform_VkImageCreateInfo(thisPtr, pCreateInfos, count, arg4);
  }
}

static bool HookLibRenderer(HMODULE hMod)
{
  if(!hMod || s_Orig_transform_VkImageCreateInfo)
    return false;

  uint8_t *base = (uint8_t *)hMod;
  void *targetFunc = NULL;

  // 1. Check known RVA 0x24d240 (prologue: 55 41 57 = push rbp; push r15; push r14)
  uint8_t *known = base + 0x24d240;
  if(known[0] == 0x55 && known[1] == 0x41 && known[2] == 0x57)
  {
    targetFunc = known;
    RDCLOG(
        "[MuMuGLESInterceptor] Found transformImpl_VkImageCreateInfo_tohost at known RVA 0x24d240 "
        "(%p)",
        targetFunc);
  }
  else
  {
    // 2. Dynamic resolution via PE section and .pdata binary search
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)base;
    if(dos->e_magic == IMAGE_DOS_SIGNATURE)
    {
      PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
      if(nt->Signature == IMAGE_NT_SIGNATURE)
      {
        DWORD pdataVA =
            nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
        DWORD pdataSize = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;

        const char *searchStr = "transformImpl_VkImageCreateInfo_tohost";
        size_t searchLen = strlen(searchStr);

        PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
        uint8_t *rdata = NULL;
        DWORD rdataVA = 0, rdataSize = 0;
        uint8_t *text = NULL;
        DWORD textVA = 0, textSize = 0;

        for(WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
        {
          if(memcmp(sec[i].Name, ".rdata", 6) == 0)
          {
            rdata = base + sec[i].VirtualAddress;
            rdataVA = sec[i].VirtualAddress;
            rdataSize = sec[i].Misc.VirtualSize;
          }
          else if(memcmp(sec[i].Name, ".text", 5) == 0)
          {
            text = base + sec[i].VirtualAddress;
            textVA = sec[i].VirtualAddress;
            textSize = sec[i].Misc.VirtualSize;
          }
        }

        if(rdata && text && pdataVA && pdataSize)
        {
          uint8_t *strPtr = NULL;
          for(DWORD i = 0; i + searchLen <= rdataSize; i++)
          {
            if(memcmp(rdata + i, searchStr, searchLen) == 0)
            {
              strPtr = rdata + i;
              break;
            }
          }

          if(strPtr)
          {
            DWORD strRVA = rdataVA + (DWORD)(strPtr - rdata);
            for(DWORD i = 0; i + 4 <= textSize; i++)
            {
              DWORD instrRVA = textVA + i;
              int32_t disp = *(int32_t *)(text + i);
              if((DWORD)(instrRVA + 4 + disp) == strRVA)
              {
                PRUNTIME_FUNCTION funcs = (PRUNTIME_FUNCTION)(base + pdataVA);
                int count = (int)(pdataSize / sizeof(RUNTIME_FUNCTION));
                int low = 0, high = count - 1;
                while(low <= high)
                {
                  int mid = (low + high) / 2;
                  if(funcs[mid].BeginAddress <= instrRVA && instrRVA < funcs[mid].EndAddress)
                  {
                    targetFunc = base + funcs[mid].BeginAddress;
                    RDCLOG(
                        "[MuMuGLESInterceptor] Dynamically resolved transform function via pdata: "
                        "%p (RVA 0x%x)",
                        targetFunc, funcs[mid].BeginAddress);
                    break;
                  }
                  else if(instrRVA < funcs[mid].BeginAddress)
                  {
                    high = mid - 1;
                  }
                  else
                  {
                    low = mid + 1;
                  }
                }
                break;
              }
            }
          }
        }
      }
    }
  }

  if(targetFunc)
  {
    s_Orig_transform_VkImageCreateInfo = (PFN_transform_VkImageCreateInfo)InstallRobustHook19(
        targetFunc, (void *)&Hooked_transform_VkImageCreateInfo);
    RDCLOG("[MuMuGLESInterceptor] Hooked libRenderer.dll transform function successfully at %p!",
           targetFunc);
    DiagLog("[MuMuGLESInterceptor] Hooked libRenderer.dll transform function successfully!");
    return true;
  }

  RDCERR("[MuMuGLESInterceptor] Failed to locate transform function in libRenderer.dll");
  DiagLog("[MuMuGLESInterceptor] Failed to locate transform function in libRenderer.dll");
  return false;
}

// GLES hooks (fallback if GLES is used)
typedef void(APIENTRY *PFN_glCompressedTexImage2D)(uint32_t target, int32_t level,
                                                   uint32_t internalformat, int32_t width,
                                                   int32_t height, int32_t border,
                                                   int32_t imageSize, const void *data);

typedef void(APIENTRY *PFN_glCompressedTexSubImage2D)(uint32_t target, int32_t level,
                                                      int32_t xoffset, int32_t yoffset,
                                                      int32_t width, int32_t height,
                                                      uint32_t format, int32_t imageSize,
                                                      const void *data);

static PFN_glCompressedTexImage2D s_Orig_glCompressedTexImage2D = NULL;
static PFN_glCompressedTexSubImage2D s_Orig_glCompressedTexSubImage2D = NULL;

static void APIENTRY Hooked_glCompressedTexImage2D(uint32_t target, int32_t level,
                                                   uint32_t internalformat, int32_t width,
                                                   int32_t height, int32_t border,
                                                   int32_t imageSize, const void *data)
{
  MuMuGLESInterceptor::Inst().RecordCompressedTex(target, level, internalformat, width, height,
                                                  (uint32_t)imageSize);

  if(s_Orig_glCompressedTexImage2D)
  {
    s_Orig_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize,
                                  data);
  }
}

static void APIENTRY Hooked_glCompressedTexSubImage2D(uint32_t target, int32_t level,
                                                      int32_t xoffset, int32_t yoffset,
                                                      int32_t width, int32_t height,
                                                      uint32_t format, int32_t imageSize,
                                                      const void *data)
{
  if(level == 0 || (xoffset == 0 && yoffset == 0))
  {
    MuMuGLESInterceptor::Inst().RecordCompressedTex(target, level, format, width, height,
                                                    (uint32_t)imageSize);
  }

  if(s_Orig_glCompressedTexSubImage2D)
  {
    s_Orig_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format,
                                     imageSize, data);
  }
}

static bool HookModuleDirect(HMODULE hMod, const char *modName)
{
  void *pFunc1 = (void *)GetProcAddress(hMod, "glCompressedTexImage2D");
  void *pFunc2 = (void *)GetProcAddress(hMod, "glCompressedTexSubImage2D");

  if(pFunc1 && pFunc1 != (void *)&Hooked_glCompressedTexImage2D)
  {
    RDCLOG(
        "[MuMuGLESInterceptor] Installing direct in-memory hook on module '%s' at %p (SubImage at "
        "%p)",
        modName, pFunc1, pFunc2);

    s_Orig_glCompressedTexImage2D =
        (PFN_glCompressedTexImage2D)InstallRobustHook(pFunc1, (void *)&Hooked_glCompressedTexImage2D);

    if(pFunc2)
    {
      s_Orig_glCompressedTexSubImage2D = (PFN_glCompressedTexSubImage2D)InstallRobustHook(
          pFunc2, (void *)&Hooked_glCompressedTexSubImage2D);
    }

    RDCLOG("[MuMuGLESInterceptor] Direct in-memory hook active for '%s'!", modName);
    return true;
  }
  return false;
}

// Background scanner thread to auto-detect and hook libRenderer and GLES modules
static DWORD WINAPI GLESScanThread(LPVOID lpParam)
{
  RDCLOG("[MuMuGLESInterceptor] Background scanner started.");
  DiagLog("[GLESScanThread] Background scanner started.");

  for(int attempt = 0; attempt < 60; attempt++)
  {
    // Try hooking libRenderer.dll if not yet hooked
    HMODULE hRend = GetModuleHandleA("libRenderer.dll");
    if(hRend && !s_Orig_transform_VkImageCreateInfo)
    {
      if(HookLibRenderer(hRend))
      {
        DiagLog("[GLESScanThread] Hooked libRenderer.dll in background scan loop.");
      }
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if(hSnap != INVALID_HANDLE_VALUE)
    {
      MODULEENTRY32W me32;
      me32.dwSize = sizeof(MODULEENTRY32W);

      if(Module32FirstW(hSnap, &me32))
      {
        if(attempt == 0)
        {
          DiagLog("[GLESScanThread] === Loaded modules (attempt 0) ===");
          MODULEENTRY32W me32b;
          me32b.dwSize = sizeof(MODULEENTRY32W);
          HANDLE hSnap2 = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
          if(hSnap2 != INVALID_HANDLE_VALUE)
          {
            if(Module32FirstW(hSnap2, &me32b))
            {
              do
              {
                rdcstr n = StringFormat::Wide2UTF8(me32b.szModule);
                void *p = (void *)GetProcAddress(me32b.hModule, "glCompressedTexImage2D");
                char buf[512];
                if(p)
                  ::snprintf(buf, sizeof(buf), "  [HAS glCompressedTexImage2D] %s @ %p",
                                         n.c_str(), p);
                else
                  ::snprintf(buf, sizeof(buf), "  %s", n.c_str());
                DiagLog(buf);
              } while(Module32NextW(hSnap2, &me32b));
            }
            CloseHandle(hSnap2);
          }
          DiagLog("[GLESScanThread] === End module list ===");
        }

        do
        {
          rdcstr modName = StringFormat::Wide2UTF8(me32.szModule);
          if(modName == "libRenderer.dll" && !s_Orig_transform_VkImageCreateInfo)
          {
            HookLibRenderer(me32.hModule);
          }
          HookModuleDirect(me32.hModule, modName.c_str());
        } while(Module32NextW(hSnap, &me32));
      }
      CloseHandle(hSnap);
    }

    if(s_Orig_transform_VkImageCreateInfo && s_Orig_glCompressedTexImage2D)
      break;

    Sleep(500);
  }

  return 0;
}

MuMuGLESInterceptor &MuMuGLESInterceptor::Inst()
{
  static MuMuGLESInterceptor instance;
  return instance;
}

MuMuGLESInterceptor::MuMuGLESInterceptor()
{
}

MuMuGLESInterceptor::~MuMuGLESInterceptor()
{
  Shutdown();
}

void MuMuGLESInterceptor::Initialise()
{
  if(m_InitAttempted)
    return;

  m_InitAttempted = true;
  RDCLOG("[MuMuGLESInterceptor] Initialising GLES & Vulkan compressed texture interceptor...");
  DiagLog("[MuMuGLESInterceptor] Initialising GLES & Vulkan compressed texture interceptor...");

  // Check if libRenderer.dll is already loaded
  HMODULE hRenderer = GetModuleHandleA("libRenderer.dll");
  if(hRenderer)
  {
    if(HookLibRenderer(hRenderer))
      m_RendererHooked = true;
  }

  // Check if libGLESv2_nemu.dll or known libs are already loaded in memory right now
  const char *knownLibs[] = {"libGLESv2_nemu.dll", "libGLESv2.dll", "libGLES_CM.dll", "GLESv2.dll",
                             "vulkan_gles.dll"};
  for(const char *lib : knownLibs)
  {
    HMODULE hMod = GetModuleHandleA(lib);
    if(hMod)
    {
      if(HookModuleDirect(hMod, lib))
        m_Hooked = true;
    }
  }

  // Also launch background scanner to detect any DLL loaded asynchronously
  HANDLE hThread = CreateThread(NULL, 0, GLESScanThread, NULL, 0, NULL);
  if(hThread)
    CloseHandle(hThread);
}

void MuMuGLESInterceptor::Shutdown()
{
  SCOPED_LOCK(m_Lock);
  m_PendingQueue.clear();
  m_ActiveManifest.clear();
}

void MuMuGLESInterceptor::RecordVulkanCompressedTex(uint32_t vkFormat, uint32_t width,
                                                   uint32_t height, uint32_t mips)
{
  SCOPED_LOCK(m_Lock);

  CompressedTexMetadata meta;
  meta.width = width;
  meta.height = height;
  meta.level = mips;
  meta.internalformat = vkFormat;
  meta.imageSize = CalcVulkanCompressedBytes(vkFormat, width, height, mips);
  meta.uncompressedSize = CalcVulkanUncompressedBytes(width, height, mips);
  meta.timestamp = Timing::GetTick();
  meta.isVulkan = true;

  if(m_PendingQueue.size() >= 4096)
    m_PendingQueue.erase(0);
  m_PendingQueue.push_back(meta);

  if(m_AllIntercepted.size() >= 8192)
    m_AllIntercepted.erase(0);
  m_AllIntercepted.push_back(meta);

  char diagBuf[256];
  ::snprintf(
      diagBuf, sizeof(diagBuf),
      "[MuMuGLESInterceptor] Intercepted Vulkan %s %ux%u (Mips %u, %u KB -> %u KB, %.1f%% saved)",
      meta.GetFormatString(), width, height, mips, meta.imageSize / 1024,
      meta.uncompressedSize / 1024, (1.0f - meta.GetCompressionRatio()) * 100.0f);
  RDCLOG("%s", diagBuf);
  DiagLog(diagBuf);
}

struct VideoStreamTracker
{
  uint64_t lastTick = 0;
  uint32_t frameCount = 0;
  float smoothedFps = 0.0f;
};
static std::map<uint64_t, VideoStreamTracker> s_VideoStreams;

void MuMuGLESInterceptor::RecordVideoFrameTex(uint32_t width, uint32_t height, const char *codec,
                                             const char *pixelFormat, uint32_t halFormat)
{
  SCOPED_LOCK(m_Lock);

  CompressedTexMetadata meta;
  meta.width = width;
  meta.height = height;
  meta.level = 1;
  meta.internalformat = halFormat;
  // NV12 / YUV420 standard is 1.5 bytes per pixel (12 bpp)
  meta.imageSize = (width * height * 3) / 2;
  // Host PC Vulkan standard uncompressed RGBA8 is 4 bytes per pixel (32 bpp)
  meta.uncompressedSize = width * height * 4;
  meta.timestamp = Timing::GetTick();
  meta.isVulkan = true;
  meta.isVideo = true;
  meta.videoCodec = codec ? codec : "MediaCodec_Video";
  meta.videoPixelFormat = pixelFormat ? pixelFormat : "NV12 (YUV420)";

  // Stream tracking for FPS and frame counter
  uint64_t streamKey = ((uint64_t)width << 32) | (uint64_t)height;
  VideoStreamTracker &tracker = s_VideoStreams[streamKey];
  tracker.frameCount++;

  if(tracker.lastTick > 0 && meta.timestamp > tracker.lastTick)
  {
    double freq = (double)Timing::GetTickFrequency();
    double deltaSec = (double)(meta.timestamp - tracker.lastTick) / freq;
    if(deltaSec > 0.005 && deltaSec < 1.0)
    {
      float instantFps = (float)(1.0 / deltaSec);
      if(tracker.smoothedFps <= 0.0f)
        tracker.smoothedFps = instantFps;
      else
        tracker.smoothedFps = tracker.smoothedFps * 0.7f + instantFps * 0.3f;
    }
  }
  tracker.lastTick = meta.timestamp;

  meta.videoFps = tracker.smoothedFps;
  meta.videoFrameCount = tracker.frameCount;

  if(m_PendingQueue.size() >= 4096)
    m_PendingQueue.erase(0);
  m_PendingQueue.push_back(meta);

  if(m_AllIntercepted.size() >= 8192)
    m_AllIntercepted.erase(0);
  m_AllIntercepted.push_back(meta);

  char diagBuf[256];
  ::snprintf(
      diagBuf, sizeof(diagBuf),
      "[MuMuGLESInterceptor] Intercepted Video Frame %s (%s) %ux%u @ %.1f FPS (#%u) (%u KB -> %u KB, %.1f%% saved)",
      meta.videoCodec.c_str(), meta.videoPixelFormat.c_str(), width, height,
      meta.videoFps, meta.videoFrameCount,
      meta.imageSize / 1024, meta.uncompressedSize / 1024,
      (1.0f - meta.GetCompressionRatio()) * 100.0f);
  RDCLOG("%s", diagBuf);
  DiagLog(diagBuf);
}

void MuMuGLESInterceptor::RecordCompressedTex(uint32_t target, int32_t level,
                                             uint32_t internalformat, int32_t width,
                                             int32_t height, uint32_t imageSize)
{
  SCOPED_LOCK(m_Lock);

  CompressedTexMetadata meta;
  meta.width = (uint32_t)width;
  meta.height = (uint32_t)height;
  meta.level = (uint32_t)level;
  meta.internalformat = internalformat;
  meta.imageSize = imageSize;
  meta.uncompressedSize = (uint32_t)(width * height * 4);
  meta.timestamp = Timing::GetTick();
  meta.isVulkan = false;

  if(level > 0)
  {
    for(size_t i = 0; i < m_PendingQueue.size(); i++)
    {
      if(m_PendingQueue[i].internalformat == internalformat &&
         (m_PendingQueue[i].width >> level) == (uint32_t)width &&
         (m_PendingQueue[i].height >> level) == (uint32_t)height)
      {
        m_PendingQueue[i].imageSize += imageSize;
        break;
      }
    }
  }
  else
  {
    if(m_PendingQueue.size() >= 4096)
      m_PendingQueue.erase(0);

    m_PendingQueue.push_back(meta);
  }

  if(m_AllIntercepted.size() >= 8192)
    m_AllIntercepted.erase(0);
  m_AllIntercepted.push_back(meta);

  RDCLOG("[MuMuGLESInterceptor] Intercepted GLES %s %ux%u (Level %u, %u KB)", meta.GetFormatString(),
         width, height, level, imageSize / 1024);
}

bool MuMuGLESInterceptor::MatchCompressedTex(uint32_t width, uint32_t height,
                                             CompressedTexMetadata &outMeta)
{
  SCOPED_LOCK(m_Lock);

  // Search FIFO from oldest to newest of matching dimensions
  for(size_t i = 0; i < m_PendingQueue.size(); i++)
  {
    if(m_PendingQueue[i].width == width && m_PendingQueue[i].height == height)
    {
      outMeta = m_PendingQueue[i];
      m_PendingQueue.erase(i);
      return true;
    }
  }

  // Fallback: search in all intercepted history
  for(int i = (int)m_AllIntercepted.size() - 1; i >= 0; i--)
  {
    if(m_AllIntercepted[i].width == width && m_AllIntercepted[i].height == height)
    {
      outMeta = m_AllIntercepted[i];
      return true;
    }
  }

  return false;
}

void MuMuGLESInterceptor::RegisterVulkanImage(ResourceId resId, const CompressedTexMetadata &meta)
{
  SCOPED_LOCK(m_Lock);
  m_ActiveManifest[resId] = meta;
  m_PersistentRegistry[resId] = meta;
  RDCLOG("[MuMuGLESInterceptor] Associated Image %s with Android format: %s (%u KB)",
         ToStr(resId).c_str(), meta.GetFormatString(), meta.imageSize / 1024);
}

bool MuMuGLESInterceptor::GetImageMetadata(ResourceId resId, CompressedTexMetadata &outMeta)
{
  SCOPED_LOCK(m_Lock);
  auto it = m_ActiveManifest.find(resId);
  if(it != m_ActiveManifest.end())
  {
    outMeta = it->second;
    return true;
  }
  auto pit = m_PersistentRegistry.find(resId);
  if(pit != m_PersistentRegistry.end())
  {
    outMeta = pit->second;
    return true;
  }
  return false;
}

void MuMuGLESInterceptor::ResetFrame()
{
  SCOPED_LOCK(m_Lock);
  // Do NOT clear m_PersistentRegistry or m_AllIntercepted.
}

static void SafeWriteJSON(const rdcstr &path, const rdcstr &content)
{
  FILE *f = FileIO::fopen(path, FileIO::WriteText);
  if(f)
  {
    FileIO::fwrite(content.data(), 1, content.size(), f);
    FileIO::fclose(f);
    RDCLOG("[MuMuGLESInterceptor] Wrote manifest to: %s", path.c_str());
  }
}

void MuMuGLESInterceptor::ExportManifest(const rdcstr &capturePath)
{
  SCOPED_LOCK(m_Lock);

  const std::map<ResourceId, CompressedTexMetadata> &source =
      !m_PersistentRegistry.empty() ? m_PersistentRegistry : m_ActiveManifest;

  rdcstr json = "{\n  \"textures\": [\n";
  bool first = true;
  uint64_t totalRealVRAM = 0;
  uint64_t totalUncompressedVRAM = 0;
  std::map<uint64_t, const CompressedTexMetadata *> byDims;

  for(const auto &pair : source)
  {
    const CompressedTexMetadata &m = pair.second;
    uint32_t uncompSize = m.GetDecompressedSize();
    totalRealVRAM += m.imageSize;
    totalUncompressedVRAM += uncompSize;

    uint64_t dimKey = ((uint64_t)m.width << 32) | (uint64_t)m.height;
    byDims[dimKey] = &m;

    if(!first)
      json += ",\n";
    first = false;

    float savedPct = uncompSize > 0
                         ? (1.0f - (float)m.imageSize / (float)uncompSize) * 100.0f
                         : 0.0f;

    json += StringFormat::Fmt(
        "    {\n"
        "      \"resourceId\": \"%s\",\n"
        "      \"format\": \"%s\",\n"
        "      \"width\": %u,\n"
        "      \"height\": %u,\n"
        "      \"androidVramBytes\": %u,\n"
        "      \"pcVramBytes\": %u,\n"
        "      \"compressionRatio\": %.2f,\n"
        "      \"savingsPercent\": %.2f,\n"
        "      \"isVideo\": %s,\n"
        "      \"videoCodec\": \"%s\",\n"
        "      \"videoPixelFormat\": \"%s\",\n"
        "      \"videoFps\": %.1f,\n"
        "      \"videoFrameCount\": %u\n"
        "    }",
        ToStr(pair.first).c_str(), m.GetFormatString(), m.width, m.height, m.imageSize, uncompSize,
        m.GetCompressionRatio(), savedPct, m.isVideo ? "true" : "false",
        m.videoCodec.c_str(), m.videoPixelFormat.c_str(), m.videoFps, m.videoFrameCount);
  }

  json += "\n  ],\n";

  // Also include byDimensions map for direct resolution lookup
  json += "  \"byDimensions\": {\n";
  bool firstDim = true;
  for(auto it = byDims.begin(); it != byDims.end(); ++it)
  {
    if(!firstDim)
      json += ",\n";
    firstDim = false;

    const CompressedTexMetadata &m = *it->second;
    uint32_t uncompSize = m.GetDecompressedSize();
    float savedPct = uncompSize > 0
                         ? (1.0f - (float)m.imageSize / (float)uncompSize) * 100.0f
                         : 0.0f;

    uint32_t w = uint32_t(it->first >> 32);
    uint32_t h = uint32_t(it->first & 0xFFFFFFFF);

    json += StringFormat::Fmt(
        "    \"%ux%u\": {\n"
        "      \"format\": \"%s\",\n"
        "      \"androidVramBytes\": %u,\n"
        "      \"pcVramBytes\": %u,\n"
        "      \"savingsPercent\": %.2f,\n"
        "      \"isVideo\": %s,\n"
        "      \"videoCodec\": \"%s\",\n"
        "      \"videoPixelFormat\": \"%s\",\n"
        "      \"videoFps\": %.1f,\n"
        "      \"videoFrameCount\": %u\n"
        "    }",
        w, h, m.GetFormatString(), m.imageSize, uncompSize, savedPct,
        m.isVideo ? "true" : "false", m.videoCodec.c_str(), m.videoPixelFormat.c_str(),
        m.videoFps, m.videoFrameCount);
  }
  json += "\n  },\n";

  float overallPct = totalUncompressedVRAM > 0
                         ? (1.0f - (float)totalRealVRAM / (float)totalUncompressedVRAM) * 100.0f
                         : 0.0f;

  json += StringFormat::Fmt(
      "  \"summary\": {\n"
      "    \"totalTextures\": %zu,\n"
      "    \"totalAndroidVramBytes\": %llu,\n"
      "    \"totalPcVramBytes\": %llu,\n"
      "    \"overallSavingsPercent\": %.2f\n"
      "  }\n"
      "}\n",
      source.size(), totalRealVRAM, totalUncompressedVRAM, overallPct);

  // 1. Output next to capture file in Temp/target directory
  rdcstr manifestPath = capturePath + ".manifest.json";
  SafeWriteJSON(manifestPath, json);

  // 2. Also output to E:/Task/RDC_Res/latest.manifest.json
  SafeWriteJSON("E:/Task/RDC_Res/latest.manifest.json", json);

  // 3. If capturePath has a filename, copy to E:/Task/RDC_Res/<filename>.manifest.json
  rdcstr filename = capturePath;
  int lastSlash = (int)filename.find_last_of("/\\");
  if(lastSlash >= 0)
    filename = filename.substr(lastSlash + 1);
  if(!filename.empty())
  {
    rdcstr rdcResPath = "E:/Task/RDC_Res/" + filename + ".manifest.json";
    SafeWriteJSON(rdcResPath, json);
  }

  RDCLOG("[MuMuGLESInterceptor] Saved mobile asset manifest for '%s' (%zu textures, %.1f%% saved).",
         capturePath.c_str(), source.size(), overallPct);
}
