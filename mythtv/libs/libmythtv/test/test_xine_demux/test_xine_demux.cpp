/*
 *  Class TextXineDemux
 *
 *  Copyright (c) David Hampton 2020
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */
#include <iostream>
#include "test_xine_demux.h"
#include "captions/xine_demux_sputext.h"

void TextXineDemux::initTestCase()
{
    QDir::setCurrent("libmythtv/test/test_xine_demux");
}

void TextXineDemux::test_captions_microdvd(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/microdvd.sub");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_MICRODVD);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 17);
    QCOMPARE(sub_data.subtitles[0].lines, 1);
    QCOMPARE(sub_data.subtitles[14].lines, 2);
    QCOMPARE(sub_data.subtitles[15].lines, 2);
    QCOMPARE(sub_data.subtitles[16].lines, 1);
}

void TextXineDemux::test_captions_srt(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/subrip.srt");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_SUBRIP);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 4);
    QCOMPARE(sub_data.subtitles[3].lines, 3);
    QVERIFY(!QString(sub_data.subtitles[3].text[1]).contains("{\\i1}"));
    QVERIFY(!QString(sub_data.subtitles[3].text[1]).contains("</i>"));
}

void TextXineDemux::test_captions_subviewer_data(void)
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("format");

    QTest::newRow("subviewer")  << "samples/subviewer.sub"  << FORMAT_SUBVIEWER;
    QTest::newRow("subviewer2") << "samples/subviewer2.sub" << FORMAT_SUBVIEWER2;
}

void TextXineDemux::test_captions_subviewer(void)
{
    QFETCH(QString, filename);
    QFETCH(int, format);

    demux_sputext_t sub_data {};

    QFile file(filename);
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, format);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 4);
    QCOMPARE(sub_data.subtitles[1].lines, 2);
    QCOMPARE(sub_data.subtitles[2].lines, 3);
    QCOMPARE(sub_data.subtitles[3].lines, 1);
    QVERIFY(QString(sub_data.subtitles[2].text[0]).contains("String3"));
}

void TextXineDemux::test_captions_smi(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/sami.smi");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    // Loses first blank line.
    // Loses last line (regardless of if blank).
    QCOMPARE(sub_data.format, FORMAT_SAMI);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 4);
    QCOMPARE(sub_data.subtitles[3].lines, 3);
    QVERIFY(!QString(sub_data.subtitles[3].text[1]).contains("</i>"));
}

void TextXineDemux::test_captions_vplayer(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/vplayer.txt");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_VPLAYER);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 5);
    QCOMPARE(sub_data.subtitles[3].lines, 1);
}

void TextXineDemux::test_captions_rt(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/realtext.txt");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_RT);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 8);
}

void TextXineDemux::test_captions_ssa_ass_data(void)
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<int>("expectedLines");

    QTest::newRow("subtitles.v3.ssa")  << "samples/substationalpha.v3.ssa" << 5;
    QTest::newRow("subtitles.v4.ssa")  << "samples/substationalpha.v4.ssa" << 3;
    QTest::newRow("subtitles.ass")     << "samples/advancedssa.ass"        << 3;
}

void TextXineDemux::test_captions_ssa_ass(void)
{
    QFETCH(QString, filename);
    QFETCH(int, expectedLines);

    demux_sputext_t sub_data {};

    QFile file(filename);
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    // Loses first blank line.
    // Loses last line (regardless of if blank).
    QCOMPARE(sub_data.format, FORMAT_SSA);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, expectedLines);
}

// Phoenix Japanimation Society
void TextXineDemux::test_captions_pjs(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/phoenixjapanimationsociety.pjs");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_PJS);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 2);
}

void TextXineDemux::test_captions_mpsub(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/mpsub.sub");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_MPSUB);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 3);
    QCOMPARE(sub_data.subtitles[0].lines, 1);
    QCOMPARE(sub_data.subtitles[1].lines, 2);
}

void TextXineDemux::test_captions_aqtitle(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/aqtitles.aqt.sub");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_AQTITLE);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 6);
    QCOMPARE(sub_data.subtitles[0].lines, 2);
    QCOMPARE(sub_data.subtitles[4].lines, 2);
    QCOMPARE(sub_data.subtitles[5].lines, 1);
}

void TextXineDemux::test_captions_jaco(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/jaco.sub");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_JACOBSUB);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 37);
    // Lines containing the "RLB" directive are skipped, altering the
    // line numbers compared to the original file.
    QCOMPARE(sub_data.subtitles[0].lines, 3);
    QCOMPARE(sub_data.subtitles[4].lines, 1);
    QVERIFY(QString(sub_data.subtitles[4].text[0]).startsWith("(And"));
    QCOMPARE(sub_data.subtitles[5].lines, 3);
    QCOMPARE(sub_data.subtitles[6].lines, 2);
}

void TextXineDemux::test_captions_subrip09(void)
{
    demux_sputext_t sub_data {};

    QFile file("samples/subrip09.srt");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_SUBRIP09);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 4);
    QCOMPARE(sub_data.subtitles[2].lines, 3);
    QCOMPARE(sub_data.subtitles[3].lines, 2);
    QVERIFY(QString(sub_data.subtitles[3].text[1]).contains("fleece"));
}

void TextXineDemux::test_captions_mpl2(void) // MPL
{
    demux_sputext_t sub_data {};

    QFile file("samples/mpl2.mpl.sub");
    QCOMPARE(file.exists(), true);
    QCOMPARE(file.open(QIODevice::ReadOnly | QIODevice::Text), true);

    QByteArray ba = file.readAll();
    sub_data.rbuffer_text = ba.data();
    sub_data.rbuffer_len = ba.size();

    subtitle_t *loaded_subs = sub_read_file(&sub_data);

    QCOMPARE(sub_data.format, FORMAT_MPL2);
    QVERIFY(loaded_subs != nullptr);
    QCOMPARE(sub_data.num, 6);
    QCOMPARE(sub_data.subtitles[0].lines, 1);
    QCOMPARE(sub_data.subtitles[1].lines, 2);
    QCOMPARE(sub_data.subtitles[2].lines, 1);
    QCOMPARE(sub_data.subtitles[5].lines, 2);
}

void TextXineDemux::cleanupTestCase()
{
}

QTEST_APPLESS_MAIN(TextXineDemux)
