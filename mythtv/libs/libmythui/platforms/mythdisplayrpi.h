#ifndef MYTHDISPLAYRPI_H
#define MYTHDISPLAYRPI_H

// Qt
#include <QMap>
#include <QWaitCondition>

// MythTV
#include "mythdisplay.h"

// Broadcom
extern "C" {
#include "interface/vmcs_host/vc_tvservice.h"
}

class MythDisplayRPI : public MythDisplay
{
    Q_OBJECT

  public:
    MythDisplayRPI();
   ~MythDisplayRPI() override;

    void  UpdateCurrentMode (void) override final;
    bool  UsingVideoModes   (void) override final;
    bool  SwitchToVideoMode (QSize Size, double Framerate) override final;
    const vector<MythDisplayMode>& GetVideoModes(void) override final;
    void  Callback(uint32_t Reason, uint32_t Param1, uint32_t Param2);

  private:
    void  GetEDID(void);

    QMutex          m_modeChangeLock   { };
    QWaitCondition  m_modeChangeWait   { };
    VCHI_INSTANCE_T m_vchiInstance     { nullptr };
    int             m_deviceId         { -1 };
    QMap<uint64_t, QPair<uint32_t, uint32_t> > m_modeMap { };
};

#endif // MYTHDISPLAYRPI_H
