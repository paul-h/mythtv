#ifndef MYTHVTBINTEROP_H
#define MYTHVTBINTEROP_H

// MythTV
#include "opengl/mythopenglinterop.h"

// OSX
#include <CoreVideo/CoreVideo.h>

class MythVTBInterop : public MythOpenGLInterop
{
  public:
    static void GetVTBTypes(MythRenderOpenGL* Render, MythInteropGPU::InteropMap& Types);
    static MythVTBInterop* CreateVTB(MythRenderOpenGL* Context);
    vector<MythVideoTextureOpenGL*> Acquire(MythRenderOpenGL*     Context,
                                            MythVideoColourSpace* ColourSpace,
                                            MythVideoFrame*       Frame,
                                            FrameScanType         Scan) override;

  protected:
    CVPixelBufferRef Verify(MythRenderOpenGL* Context, MythVideoColourSpace* ColourSpace,
                            MythVideoFrame* Frame);
    MythVTBInterop(MythRenderOpenGL *Context, MythOpenGLInterop::InteropType Type);
   ~MythVTBInterop() override;
};

class MythVTBSurfaceInterop : public MythVTBInterop
{
  public:
    explicit MythVTBSurfaceInterop(MythRenderOpenGL *Context);
   ~MythVTBSurfaceInterop() override;

    vector<MythVideoTextureOpenGL*> Acquire(MythRenderOpenGL*     Context,
                                            MythVideoColourSpace* ColourSpace,
                                            MythVideoFrame*       Frame,
                                            FrameScanType         Scan) override;

  private:

    void RotateReferenceFrames(IOSurfaceID Buffer);
    vector<MythVideoTextureOpenGL*> GetReferenceFrames(void);
    QVector<IOSurfaceID> m_referenceFrames { };
};

#endif
