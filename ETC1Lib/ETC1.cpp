#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rg_etc1.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

#define SWAP64(val) ((unsigned long long)( \
	(((unsigned long long)(val) & (unsigned long long)0x00000000000000FF) << 56) | \
	(((unsigned long long)(val) & (unsigned long long)0x000000000000FF00) << 40) | \
	(((unsigned long long)(val) & (unsigned long long)0x0000000000FF0000) << 24) | \
	(((unsigned long long)(val) & (unsigned long long)0x00000000FF000000) <<  8) | \
	(((unsigned long long)(val) & (unsigned long long)0x000000FF00000000) >>  8) | \
	(((unsigned long long)(val) & (unsigned long long)0x0000FF0000000000) >> 24) | \
	(((unsigned long long)(val) & (unsigned long long)0x00FF000000000000) >> 40) | \
	(((unsigned long long)(val) & (unsigned long long)0xFF00000000000000) >> 56)))

extern "C" __declspec (dllexport) void ConvertETC1(unsigned int* dataOut, unsigned int* dataOutSize, unsigned int* dataIn, unsigned short width, unsigned short height, bool alpha)
{
	unsigned int bufferSize = (sizeof(unsigned int) * width * height);

	if (dataOut == NULL)
	{
		*dataOutSize = bufferSize;
		return;
	}

	unsigned int* buffer = (unsigned int*)malloc(bufferSize);
	unsigned int xImage, yImage, inOffset = 0;

	for(xImage = 0; xImage < width; xImage += 8)
	{
		for(yImage = 0; yImage < height; yImage += 8)
		{
			unsigned long long data;
			unsigned int pixels[4 * 4];
			unsigned int x, y, z, xStart, yStart;
			unsigned int* ptr;

			if(alpha)
			{
				for(z = 0; z < 4; z++)
				{
					xStart = (z == 0 || z == 2 ? 0 : 4);
					yStart = (z == 0 || z == 1 ? 0 : 4);

					memcpy(&data, &dataIn[inOffset], sizeof(unsigned long long));
					inOffset += 2;

					for(y = yImage + xStart; y < yImage + xStart + 4; y++)
						for(x = xImage + yStart; x < xImage + yStart + 4; x++)
						{
							buffer[(x * height) + y] = (((data & 0xF) << 28) & 0xFF000000);
							data >>= 4;
						}

						xStart = (z == 0 || z == 1 ? 0 : 4);
						yStart = (z == 0 || z == 2 ? 0 : 4);

						memcpy(&data, &dataIn[inOffset], sizeof(unsigned long long));
						inOffset += 2;

						data = SWAP64(data);
						rg_etc1::unpack_etc1_block(&data, pixels, false);
						ptr = pixels;

						for(x = xImage + xStart; x < xImage + xStart + 4; x++)
							for(y = yImage + yStart; y < yImage + yStart + 4; y++)
								buffer[(x * height) + y] |= (*(ptr++) & 0xFFFFFF);
				}
			}
			else
			{
				for(z = 0; z < 4; z++)
				{
					xStart = (z == 0 || z == 1 ? 0 : 4);
					yStart = (z == 0 || z == 2 ? 0 : 4);

					memcpy(&data, &dataIn[inOffset], sizeof(unsigned long long));
					inOffset += 2;

					data = SWAP64(data);
					rg_etc1::unpack_etc1_block(&data, pixels, false);
					ptr = pixels;

					for(x = xImage + xStart; x < xImage + xStart + 4; x++)
						for(y = yImage + yStart; y < yImage + yStart + 4; y++)
							buffer[(x * height) + y] = *(ptr++);
				}
			}
		}
	}

	if (dataOut == NULL)
	{
		*dataOutSize = bufferSize;
	}
	else
	{
		*dataOutSize = MIN(bufferSize, *dataOutSize);
		::memcpy(dataOut, buffer, *dataOutSize);
	}

	free(buffer);
}
