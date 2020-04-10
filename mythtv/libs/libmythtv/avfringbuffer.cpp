// MythTV
#include "avfringbuffer.h"
#include "mythcorecontext.h"

URLProtocol AVFRingBuffer::s_avfrURL;

AVFRingBuffer::AVFRingBuffer(RingBuffer *Buffer)
  : m_ringBuffer(Buffer)
{
}

void AVFRingBuffer::SetRingBuffer(RingBuffer *Buffer)
{
    m_ringBuffer = Buffer;
}

RingBuffer* AVFRingBuffer::GetRingBuffer(void)
{
    return m_ringBuffer;
}

int AVFRingBuffer::Open(URLContext *Context, const char*, int)
{
    Context->priv_data = nullptr;
    return 0;
}

int AVFRingBuffer::Read(URLContext *Context, uint8_t *Buffer, int Size)
{
    auto *avfr = reinterpret_cast<AVFRingBuffer*>(Context->priv_data);
    if (!avfr)
        return 0;

    int ret = avfr->GetRingBuffer()->Read(Buffer, Size);

    if (ret == 0)
        ret = AVERROR_EOF;
    return ret;
}

int AVFRingBuffer::Write(URLContext *h, const uint8_t *Buffer, int Size)
{
    auto *avfr = reinterpret_cast<AVFRingBuffer*>(h->priv_data);
    if (!avfr)
        return 0;

    return avfr->GetRingBuffer()->Write(Buffer, static_cast<uint>(Size));
}

int64_t AVFRingBuffer::Seek(URLContext *Context, int64_t Offset, int Whence)
{
    auto *avfr = reinterpret_cast<AVFRingBuffer*>(Context->priv_data);
    if (!avfr)
        return 0;

    if (Whence == AVSEEK_SIZE)
        return avfr->GetRingBuffer()->GetRealFileSize();

    if (Whence == SEEK_END)
        return avfr->GetRingBuffer()->GetRealFileSize() + Offset;

    return avfr->GetRingBuffer()->Seek(Offset, Whence);
}

int AVFRingBuffer::Close(URLContext*)
{
    return 0;
}

int AVFRingBuffer::WritePacket(void *Context, uint8_t *Buffer, int Size)
{
    if (!Context)
        return 0;
    return ffurl_write(reinterpret_cast<URLContext*>(Context), Buffer, Size);
}

int AVFRingBuffer::ReadPacket(void *Context, uint8_t *Buffer, int Size)
{
    if (!Context)
        return 0;
    return ffurl_read(reinterpret_cast<URLContext*>(Context), Buffer, Size);
}

int64_t AVFRingBuffer::SeekPacket(void *Context, int64_t Offset, int Whence)
{
    if (!Context)
        return 0;
    return ffurl_seek(reinterpret_cast<URLContext*>(Context), Offset, Whence);
}

URLProtocol *AVFRingBuffer::GetURLProtocol(void)
{
    static QMutex s_avringbufferLock(QMutex::Recursive);
    static bool   s_avringbufferInitialised = false;

    QMutexLocker lock(&s_avringbufferLock);
    if (!s_avringbufferInitialised)
    {
        // just in case URLProtocol's members do not have default constructor
        memset(static_cast<void*>(&s_avfrURL), 0, sizeof(s_avfrURL));
        s_avfrURL.name            = "rbuffer";
        s_avfrURL.url_open        = Open;
        s_avfrURL.url_read        = Read;
        s_avfrURL.url_write       = Write;
        s_avfrURL.url_seek        = Seek;
        s_avfrURL.url_close       = Close;
        s_avfrURL.priv_data_size  = 0;
        s_avfrURL.flags           = URL_PROTOCOL_FLAG_NETWORK;
        s_avringbufferInitialised = true;
    }
    return &s_avfrURL;
}

void AVFRingBuffer::SetInInit(bool State)
{
    m_initState = State;
    GetRingBuffer()->SetReadInternalMode(State);
}

bool AVFRingBuffer::IsInInit(void)
{
    return m_initState;
}
