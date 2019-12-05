#ifndef MYTHDISPLAY_H
#define MYTHDISPLAY_H

// Qt
#include <QSize>
#include <QScreen>
#include <QMutex>

// MythTV
#include "mythuiexp.h"
#include "referencecounter.h"
#include "DisplayResScreen.h"

// Std
#include <cmath>

#define VALID_RATE(rate) ((rate) > 20.0F && (rate) < 200.0F)

class DisplayInfo
{
  public:
    DisplayInfo(void) = default;
    explicit DisplayInfo(int Rate)
      : m_rate(Rate)
    {
    }

    int Rate(void) const
    {
        return static_cast<int>(lroundf(m_rate));
    }

    QSize m_size { 0, 0};
    QSize m_res  { 0, 0};
    float m_rate { -1 };
};

class MUI_PUBLIC MythDisplay : public QObject, public ReferenceCounter
{
    Q_OBJECT

    friend class MythMainWindow;

  public:
    static MythDisplay* AcquireRelease(bool Acquire = true);

    typedef enum
    {
        GUI          = 0,
        VIDEO        = 1,
        CUSTOM_GUI   = 2,
        CUSTOM_VIDEO = 3,
        DESKTOP      = 4,
        MAX_MODES    = 5,
    } Mode;

    QScreen* GetCurrentScreen (void);
    int      GetScreenCount   (void);
    double   GetPixelAspectRatio(void);

    virtual DisplayInfo GetDisplayInfo(int VideoRate = 0);
    static bool         SpanAllScreens(void);
    static QString      GetExtraScreenInfo(QScreen *qScreen);

    virtual bool UsingVideoModes   (void) { return false; }
    virtual const DisplayResVector& GetVideoModes(void);
    void         SwitchToDesktop   (void);
    bool         SwitchToGUI       (Mode NextMode = GUI);
    bool         SwitchToVideo     (int Width, int Height, double Rate = 0.0);
    QSize        GetResolution     (void);
    QSize        GetPhysicalSize   (void);
    double       GetRefreshRate    (void);
    double       GetAspectRatio    (void);
    double       EstimateVirtualAspectRatio(void);
    std::vector<double> GetRefreshRates(int Width, int Height);


  public slots:
    void ScreenChanged        (QScreen *qScreen);
    void PrimaryScreenChanged (QScreen *qScreen);
    void ScreenAdded          (QScreen *qScreen);
    void ScreenRemoved        (QScreen *qScreen);

  signals:
    void CurrentScreenChanged (QScreen *Screen);
    void ScreenCountChanged   (int Screens);

  protected:
    MythDisplay();
    virtual ~MythDisplay();

    void         SetWidget          (QWidget *MainWindow);
    QScreen*     GetDesiredScreen   (void);
    static void  DebugScreen        (QScreen *qScreen, const QString &Message);
    static float SanitiseRefreshRate(int Rate);

    void         InitialiseModes    (void);
    virtual bool SwitchToVideoMode  (int Width, int Height, double Framerate);

    QWidget* m_widget { nullptr };
    QScreen* m_screen { nullptr };
    mutable std::vector<DisplayResScreen> m_videoModes { };

  private:
    Q_DISABLE_COPY(MythDisplay)
    void PauseForModeSwitch(void);

    Mode             m_curMode            { GUI };
    DisplayResScreen m_mode[MAX_MODES]    { };
    DisplayResScreen m_last               { }; // mirror of mode[current_mode]
    DisplayResMap    m_inSizeToOutputMode { };
};

#endif // MYTHDISPLAY_H
