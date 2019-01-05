#ifndef _OPENGL_VIDEO_H__
#define _OPENGL_VIDEO_H__

#include <vector>
#include <map>
using std::vector;
using std::map;

#include <QRect>

#include "videooutbase.h"
#include "videoouttypes.h"
#include "mythrender_opengl.h"
#include "mythavutil.h"
#include "util-opengl.h"

class OpenGLFilter;

class OpenGLVideo
{
    enum OpenGLFilterType
    {
        kGLFilterNone = 0,

        // Conversion filters
        kGLFilterYUV2RGB,
        kGLFilterYV12RGB,

        // Frame scaling/resizing filters
        kGLFilterResize,
        kGLFilterBicubic,
    };
    typedef map<OpenGLFilterType,OpenGLFilter*> glfilt_map_t;

  public:
    OpenGLVideo();
   ~OpenGLVideo();

    bool Init(MythRenderOpenGL *glcontext, VideoColourSpace *colourspace,
              QSize videoDim, QSize videoDispDim, QRect displayVisibleRect,
              QRect displayVideoRect, QRect videoRect,
              bool viewport_control,  QString options,
              bool hwaccel);

    uint GetInputTexture(void) const;
    uint GetTextureType(void) const;
    void SetInputUpdated(void);
    void UpdateInputFrame(const VideoFrame *frame, bool soft_bob = false);

    /// \brief Public interface to AddFilter(OpenGLFilterType filter)
    bool AddFilter(const QString &filter)
         { return AddFilter(StringToFilter(filter)); }
    bool RemoveFilter(const QString &filter)
         { return RemoveFilter(StringToFilter(filter)); }

    bool AddDeinterlacer(const QString &deinterlacer);
    void SetDeinterlacing(bool deinterlacing);
    QString GetDeinterlacer(void) const
         { return hardwareDeinterlacer; }
    void SetSoftwareDeinterlacer(const QString &filter);

    void PrepareFrame(bool topfieldfirst, FrameScanType scan,
                      bool softwareDeinterlacing,
                      long long frame, StereoscopicMode stereo,
                      bool draw_border = false);

    void  SetMasterViewport(QSize size)   { masterViewportSize = size; }
    QSize GetViewPort(void)         const { return viewportSize; }
    void  SetVideoRect(const QRect &dispvidrect, const QRect &vidrect);
    QSize GetVideoSize(void)        const { return video_dim;}

  private:
    void Teardown(void);
    void SetViewPort(const QSize &new_viewport_size);
    bool AddFilter(OpenGLFilterType filter);
    bool RemoveFilter(OpenGLFilterType filter);
    void CheckResize(bool deinterlacing, bool allow = true);
    bool OptimiseFilters(void);
    bool AddFrameBuffer(uint &framebuffer, uint &texture, QSize vid_size);
    uint AddFragmentProgram(OpenGLFilterType name,
                            QString deint = QString(),
                            FrameScanType field = kScan_Progressive);
    uint CreateVideoTexture(QSize size, QSize &tex_size);
    QString GetProgramString(OpenGLFilterType filter,
                             QString deint = QString(),
                             FrameScanType field = kScan_Progressive);
    void GetProgramStrings(QString &vertex, QString &fragment,
                           OpenGLFilterType filter,
                           QString deint = QString(),
                           FrameScanType field = kScan_Progressive);
    void CustomiseProgramString(QString &string);
    static QString FilterToString(OpenGLFilterType filter);
    static OpenGLFilterType StringToFilter(const QString &filter);
    QSize GetTextureSize(const QSize &size);
    void SetFiltering(void);

    void RotateTextures(void);
    void SetTextureFilters(vector<GLuint> *textures, int filt, int clamp);
    void DeleteTextures(vector<GLuint> *textures);
    void TearDownDeinterlacer(void);

    MythRenderOpenGL *gl_context;
    QSize          video_disp_dim;       ///< Useful video frame size e.g. 1920x1080
    QSize          video_dim;            ///< Total video frame size e.g. 1920x1088
    QSize          viewportSize;         ///< Can be removed
    QSize          masterViewportSize;   ///< Current viewport into which OpenGL is rendered
    QRect          display_visible_rect; ///< Total useful, visible rectangle
    QRect          display_video_rect;   ///< Sub-rect of display_visible_rect for video
    QRect          video_rect;           ///< Sub-rect of video_disp_dim to display (after zoom adjustments etc)
    QRect          frameBufferRect;      ///< Rect version of video_disp_dim
    QString        softwareDeinterlacer;
    QString        hardwareDeinterlacer;
    bool           hardwareDeinterlacing;///< OpenGL deinterlacing is enabled
    VideoColourSpace *colourSpace;
    bool           viewportControl;      ///< Video has control over view port
    vector<GLuint> referenceTextures;    ///< Up to 3 reference textures for filters
    vector<GLuint> inputTextures;        ///< Textures with raw video data
    QSize          inputTextureSize;     ///< Actual size of input texture(s)
    glfilt_map_t   filters;              ///< Filter stages to be applied to frame
    long long      currentFrameNum;      ///< Can be removed
    bool           inputUpdated;         ///< Input texture has been refreshed with new data
    int            refsNeeded;           ///< Number of reference textures expected
    bool           textureRects;         ///< OpenGLVideo is using rectangular textures
    uint           textureType;          ///< Texture type (e.g. GL_TEXTURE_2D or GL_TEXTURE_RECT)
    uint           helperTexture;        ///< Extra texture for bicubic filter
    OpenGLFilterType defaultUpsize;      ///< Regular or bicubic upsizing
    uint           gl_features;          ///< OR'd list of GLFeatures in use
    uint           videoTextureType;     ///< Underlying texture type for current YUV to RGB method
    bool           preferYCBCR;          ///< Use of YCbCr textures is preferred (OpenGL-Lite profile)
    MythAVCopy     m_copyCtx;            ///< Conversion context for YV12 to UYVY (OpenGL-Lite profile)
    bool           forceResize;          ///< Global setting to force a resize stage
};
#endif // _OPENGL_VIDEO_H__
