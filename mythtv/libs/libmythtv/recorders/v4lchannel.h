// -*- Mode: c++ -*-

#ifndef CHANNEL_H
#define CHANNEL_H

#include <utility>

// MythTV headers
#include "dtvchannel.h"

#ifdef USING_V4L2
#include "videodev2.h" // needed for v4l2_std_id type
#else
using v4l2_std_id = uint64_t;
#endif

using namespace std;

#define FAKE_VIDEO 0

class TVRec;

using VidModV4L2 = QMap<int,v4l2_std_id>;

/** \class V4LChannel
 *  \brief Implements tuning for TV cards using the V4L driver API,
 *         both versions 1 and 2.
 *
 *   This class supports a wide range of tuning hardware including
 *   frame grabbers (whose output requires encoding), hardware encoders,
 *   digital cameras, and non-encoding hardware which simply records
 *   pre-encoded broadcast streams.
 *
 */
class V4LChannel : public DTVChannel
{
 public:
    V4LChannel(TVRec *parent, QString videodevice,
               QString audiodevice = "")
        : DTVChannel(parent), m_device(std::move(videodevice)),
          m_audioDevice(std::move(audiodevice)) {}
    ~V4LChannel(void) override;

    bool Init(QString &startchannel, bool setchan) override; // ChannelBase

    // Commands
    bool Open(void) override; // ChannelBase
    void Close(void) override; // ChannelBase
    using DTVChannel::Tune;
    bool Tune(const DTVMultiplex &tuning) override; // DTVChannel
    bool Tune(uint64_t frequency) override; // DTVChannel
    bool Tune(const QString &freqid, int finetune) override; // DTVChannel
    bool Retune(void) override; // ChannelBase

    // Sets
    void SetFd(int fd) override; // ChannelBase
    void SetFormat(const QString &format) override; // DTVChannel
    int  SetDefaultFreqTable(const QString &name);

    // Gets
    bool IsOpen(void)       const override // ChannelBase
        { return GetFd() >= 0; }
    int  GetFd(void)        const override // ChannelBase
        { return m_videoFd; }
    QString GetDevice(void) const override // ChannelBase
        { return m_device; }
    QString GetAudioDevice(void) const { return m_audioDevice; }
    static QString GetSIStandard(void) { return "atsc"; }

    // Picture attributes.
    bool InitPictureAttributes(void) override; // ChannelBase
    int  GetPictureAttribute(PictureAttribute attr) const override; // ChannelBase
    int  ChangePictureAttribute(PictureAdjustType type,
                                PictureAttribute attr, bool up) override; // ChannelBase

  protected:
    bool IsExternalChannelChangeSupported(void) override // ChannelBase
        { return true; }

  private:
    // Helper Sets
    void SetFreqTable(int index);
    int  SetFreqTable(const QString &name) override; // ChannelBase
    bool SetInputAndFormat(int inputNum, const QString& newFmt);

    // Helper Gets
    int  GetCurrentChannelNum(const QString &channame);
    QString GetFormatForChannel(const QString& channum,
                                const QString& inputname);

    // Helper Commands
    bool InitPictureAttribute(const QString &db_col_name);
    bool InitializeInputs(void);

  private:
    // Data
    QString           m_device;
    QString           m_audioDevice;
    int               m_videoFd           {-1};
    QString           m_deviceName;
    QString           m_driverName;
    QMap<QString,int> m_pictAttrDefault;

    struct CHANLIST *m_curList            {nullptr};
    int              m_totalChannels      {0};

    bool             m_hasStreamIO        {false};
    bool             m_hasStdIO           {false};
    bool             m_hasAsyncIO         {false};
    bool             m_hasTuner           {false};
    bool             m_hasSlicedVbi       {false};

    int              m_defaultFreqTable   {1};
    int              m_inputNumV4L        {0};
    int              m_videoModeV4L2      {0};
};

#endif
