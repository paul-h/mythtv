#include "audiooutputgraph.h"

#include <climits>
#include <cmath>
#include <cstdint>

#include <QtGlobal>
#include <QImage>
#include <QByteArray>
#include <QPair>
#include <QMutexLocker>

#include "mythlogging.h"
#include "mythpainter.h"
#include "mythimage.h"
#include "compat.h"

using namespace std::chrono_literals;

#define LOC QString("AOG::%1").arg(__func__)

const int kBufferMilliSecs = 500;

/*
 * Audio data buffer
 */
class AudioOutputGraph::Buffer : protected QByteArray
{
public:
    Buffer() = default;

    // Properties
    void SetMaxSamples(unsigned samples) { m_maxSamples = samples; }
    void SetSampleRate(unsigned sample_rate) { m_sampleRate = sample_rate; }

    static inline int BitsPerChannel() { return sizeof(short) * CHAR_BIT; }
    inline int Channels() const { return m_channels; }

    inline std::chrono::milliseconds Next() const { return m_tcNext; }
    inline std::chrono::milliseconds First() const { return m_tcFirst; }

    using range_t = QPair<std::chrono::milliseconds, std::chrono::milliseconds>;
    range_t Avail(std::chrono::milliseconds timecode) const
    {
        if (timecode == 0ms || timecode == -1ms)
            timecode = m_tcNext;

        std::chrono::milliseconds tc1 = timecode - Samples2MS(m_maxSamples / 2);
        if (tc1 < m_tcFirst)
            tc1 = m_tcFirst;

        std::chrono::milliseconds tc2 = tc1 + Samples2MS(m_maxSamples);
        if (tc2 > m_tcNext)
        {
            tc2 = m_tcNext;
            if (tc2 < tc1 + Samples2MS(m_maxSamples))
            {
                tc1 = tc2 - Samples2MS(m_maxSamples);
                if (tc1 < m_tcFirst)
                    tc1 = m_tcFirst;
            }
        }
        return {tc1, tc2};
    }

    int Samples(range_t avail) const
    {
        return MS2Samples(avail.second - avail.first);
    }

    // Operations
    void Empty()
    {
        m_tcFirst = m_tcNext = 0ms;
        m_bits = m_channels = 0;
        resize(0);
    }

    void Append(const void *b, unsigned long len, std::chrono::milliseconds timecode, int channels, int bits)
    {
        if (m_bits != bits || m_channels != channels)
        {
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("(%1, %2 channels, %3 bits)")
                .arg(timecode.count()).arg(channels).arg(bits));

            Resize(channels, bits);
            m_tcNext = m_tcFirst = timecode;
        }

        unsigned samples = Bytes2Samples(len);
        std::chrono::milliseconds tcNext = timecode + Samples2MS(samples);

        if (qAbs((timecode - m_tcNext).count()) <= 1)
        {
            Append(b, len, bits);
            m_tcNext = tcNext;
        }
        else if (timecode >= m_tcFirst && tcNext <= m_tcNext)
        {
            // Duplicate
            return;
        }
        else
        {
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString(" discontinuity %1 -> %2")
                .arg(m_tcNext.count()).arg(timecode.count()));

            Resize(channels, bits);
            Append(b, len, bits);
            m_tcFirst = timecode;
            m_tcNext = tcNext;
        }

        int overflow = size() - m_sizeMax;
        if (overflow > 0)
        {
            remove(0, overflow);
            m_tcFirst = m_tcNext - Samples2MS(Bytes2Samples(m_sizeMax));
        }
    }

    const int16_t* Data16(range_t avail) const
    {
        unsigned start = MS2Samples(avail.first - m_tcFirst);
        return reinterpret_cast< const int16_t* >(constData() + start * BytesPerSample());
    }

protected:
    inline unsigned BytesPerSample() const
    {
        return m_channels * ((m_bits + 7) / 8);
    }

    inline unsigned Bytes2Samples(unsigned bytes) const
    {
        return  (m_channels && m_bits) ? bytes / BytesPerSample() : 0;
    }

    inline std::chrono::milliseconds Samples2MS(unsigned samples) const
    {
        return m_sampleRate ? std::chrono::milliseconds((samples * 1000UL + m_sampleRate - 1) / m_sampleRate) : 0ms; // round up
    }

    inline unsigned MS2Samples(std::chrono::milliseconds msec) const
    {
        return msec > 0ms ? (msec.count() * m_sampleRate) / 1000 : 0; // NB round down
    }

    void Append(const void *b, unsigned long len, int bits)
    {
        switch (bits)
        {
          case 8:
            // 8bit unsigned to 16bit signed
            {
                unsigned long cnt = len;
                int n = size();
                resize(n + sizeof(int16_t) * cnt);
                const auto *s = reinterpret_cast< const uchar* >(b);
                auto *p = reinterpret_cast< int16_t* >(data() + n);
                while (cnt--)
                    *p++ = (int16_t(*s++) - CHAR_MAX) << (16 - CHAR_BIT);
            }
            break;

          case 16:
            append( reinterpret_cast< const char* >(b), len);
            break;

          case 32:
            // 32bit float to 16bit signed
            {
                unsigned long cnt = len / sizeof(float);
                int n = size();
                resize(n + sizeof(int16_t) * cnt);
                const float f((1 << 15) - 1);
                const auto *s = reinterpret_cast< const float* >(b);
                auto *p = reinterpret_cast< int16_t* >(data() + n);
                while (cnt--)
                    *p++ = int16_t(f * *s++);
            }
            break;

          default:
            append( reinterpret_cast< const char* >(b), len);
            break;
        }
    }

private:
    void Resize(int channels, int bits)
    {
        m_bits = bits;
        m_channels = channels;
        m_sizeMax = ((m_sampleRate * kBufferMilliSecs) / 1000) * BytesPerSample();
        resize(0);
    }

private:
    unsigned m_maxSamples {0};
    unsigned m_sampleRate {44100};
    std::chrono::milliseconds m_tcFirst {0ms}, m_tcNext {0ms};
    int m_bits {0};
    int m_channels {0};
    int m_sizeMax {0};
};


/*
 * Audio graphic
 */
AudioOutputGraph::AudioOutputGraph() :
    m_buffer(new AudioOutputGraph::Buffer())
{ }

AudioOutputGraph::~AudioOutputGraph()
{
    delete m_buffer;
}

void AudioOutputGraph::SetPainter(MythPainter* painter)
{
    QMutexLocker lock(&m_mutex);
    m_painter = painter;
}

void AudioOutputGraph::SetSampleRate(unsigned sample_rate)
{
    LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("(%1)")
        .arg(sample_rate));

    QMutexLocker lock(&m_mutex);
    m_buffer->SetSampleRate(sample_rate);
}

void AudioOutputGraph::SetSampleCount(unsigned sample_count)
{
    LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("(%1)")
        .arg(sample_count));

    QMutexLocker lock(&m_mutex);
    m_buffer->SetMaxSamples(sample_count);
}

void AudioOutputGraph::prepare()
{
}

void AudioOutputGraph::add(const void *buf, unsigned long len,
                           std::chrono::milliseconds timecode, int channels, int bits)
{
    QMutexLocker lock(&m_mutex);
    m_buffer->Append(buf, len, timecode, channels, bits);
}

void AudioOutputGraph::Reset()
{
    LOG(VB_PLAYBACK, LOG_INFO, LOC);

    QMutexLocker lock(&m_mutex);
    m_buffer->Empty();
}

MythImage *AudioOutputGraph::GetImage(std::chrono::milliseconds timecode) const
{
    QMutexLocker lock(&m_mutex);
    Buffer::range_t avail = m_buffer->Avail(timecode);

    LOG(VB_PLAYBACK, LOG_INFO, LOC +
        QString("(%1) using [%2..%3] avail [%4..%5]")
        .arg(timecode.count()).arg(avail.first.count()).arg(avail.second.count())
        .arg(m_buffer->First().count()).arg(m_buffer->Next().count()) );

    const int width = m_buffer->Samples(avail);
    if (width <= 0)
        return nullptr;

    const unsigned range = 1U << AudioOutputGraph::Buffer::BitsPerChannel();
    const double threshold = 20 * log10(1.0 / range); // 16bit=-96.3296dB => ~6dB/bit
    const int height = (int)-ceil(threshold); // 96
    if (height <= 0)
        return nullptr;

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(0);

    const int channels = m_buffer->Channels();

    // Assume signed 16 bit/sample
    const int16_t *p = m_buffer->Data16(avail);

    for (int x = 0; x < width; ++x)
    {
        int left = p[0];
        int right = channels > 1 ? p[1] : left;
        p += channels;

        unsigned avg = qAbs(left) + qAbs(right);
        double db = 20 * log10( (double)(avg ? avg : 1) / range);

        int idb = (int)ceil(db);
        QRgb rgb = idb <= m_dBsilence ? qRgb(255, 255, 255)
                 : idb <= m_dBquiet   ? qRgb(  0, 255, 255)
                 : idb <= m_dBLoud    ? qRgb(  0, 255,   0)
                 : idb <= m_dbMax     ? qRgb(255, 255,   0)
                 :                      qRgb(255,   0,   0);

        int v = height - (int)(height * (db / threshold));
        if (v >= height)
            v = height - 1;
        else if (v < 0)
            v = 0;

        for (int y = 0; y <= v; ++y)
            image.setPixel(x, height - 1 - y, rgb);
    }

    auto *mi = new MythImage(m_painter);
    mi->Assign(image);
    return mi;
}
