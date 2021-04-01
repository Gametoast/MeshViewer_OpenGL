#pragma once
#include "OutputDevice.h"
#include <QImage>
#include <QColor>
#include <QVector>
#include <QFile>


QImage loadTga(QString filePath, bool &success)
{
	QImage img;
	success = true;

	// open the file
	QFile file(filePath);

	if (!file.open(QIODevice::ReadOnly))
	{
		success = false;
	}
	else
	{

		// read in the header
		quint8 ui8x18Header[19] = { 0 };
		file.read(reinterpret_cast<char*>(&ui8x18Header), sizeof(ui8x18Header) - 1);

		//get variables
		quint32 ui32BpP;
		quint32 ui32Width;
		quint32 ui32Height;
		quint32 ui32IDLength;
		quint32 ui32PicType;
		quint32 ui32PaletteLength;
		quint32 ui32Size;

		// extract all information from header
		ui32IDLength = ui8x18Header[0];
		ui32PicType = ui8x18Header[2];
		ui32PaletteLength = ui8x18Header[6] * 0x100 + ui8x18Header[5];
		ui32Width = ui8x18Header[13] * 0x100 + ui8x18Header[12];
		ui32Height = ui8x18Header[15] * 0x100 + ui8x18Header[14];
		ui32BpP = ui8x18Header[16];

		// calculate some more information
		ui32Size = ui32Width * ui32Height * ui32BpP / 8;

		// jump to the data block
		file.seek(ui32IDLength + ui32PaletteLength + 18);

		// uncompressed
		if (ui32PicType == 2 && (ui32BpP == 24 || ui32BpP == 32))
		{
			img = QImage(ui32Width, ui32Height, ui32BpP == 32 ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
			int lineWidth = ui32Width * ui32BpP / 8;

			for (int i = ui32Height - 1; i >= 0; --i)
				file.read(reinterpret_cast<char*>(img.scanLine(i)), lineWidth);

		}
		// else if compressed 24 or 32 bit
		else if (ui32PicType == 10 && (ui32BpP == 24 || ui32BpP == 32))	// compressed
		{
			OutputDevice::getInstance()->print("compressed tga is not supported by SWBF", 1);

			img = QImage(ui32Width, ui32Height, QImage::Format_RGBA8888);

			quint8 tempChunkHeader;
			quint8 tempData[5];
			unsigned int tmp_pixelIndex = 0;

			do {
				file.read(reinterpret_cast<char*>(&tempChunkHeader), sizeof(tempChunkHeader));

				if (tempChunkHeader >> 7)	// repeat count
				{
					// just use the first 7 bits
					tempChunkHeader = (quint8(tempChunkHeader << 1) >> 1);

					file.read(reinterpret_cast<char*>(&tempData), ui32BpP / 8);

					for (int i = 0; i <= tempChunkHeader; i++)
					{
						QColor color;

						if (ui32BpP == 32)
							color.setRgba(qRgba(tempData[0], tempData[1], tempData[2], tempData[3]));
						else
							color.setRgba(qRgba(tempData[0], tempData[1], tempData[2], 255));

						img.setPixel(tmp_pixelIndex % ui32Width, ui32Height - 1 - (tmp_pixelIndex / ui32Width), color.rgba());
						tmp_pixelIndex++;
					}
				}
				else						// data count
				{
					// just use the first 7 bits
					tempChunkHeader = (uint8_t(tempChunkHeader << 1) >> 1);

					for (int i = 0; i <= tempChunkHeader; i++)
					{
						file.read(reinterpret_cast<char*>(&tempData), ui32BpP / 8);

						QColor color;

						if (ui32BpP == 32)
							color.setRgba(qRgba(tempData[0], tempData[1], tempData[2], tempData[3]));
						else
							color.setRgba(qRgba(tempData[0], tempData[1], tempData[2], 255));

						img.setPixel(tmp_pixelIndex % ui32Width, ui32Height - 1 - (tmp_pixelIndex / ui32Width), color.rgba());
						tmp_pixelIndex++;
					}
				}
			} while (tmp_pixelIndex < (ui32Width * ui32Height));
		}
		// not useable format
		else
		{
			success = false;
		}
	}

	if (file.isOpen())
		file.close();
	return qMove(img).rgbSwapped();
	
}
