//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
    IVC *image = (IVC *)malloc(sizeof(IVC));

    if (image == NULL)
        return NULL;
    if ((levels <= 0) || (levels > 255))
        return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = image->width * image->channels;
    image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

    if (image->data == NULL)
    {
        return vc_image_free(image);
    }

    return image;
}

// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
    if (image != NULL)
    {
        if (image->data != NULL)
        {
            free(image->data);
            image->data = NULL;
        }

        free(image);
        image = NULL;
    }

    return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
    char *t;
    int c;

    for (;;)
    {
        while (isspace(c = getc(file)))
            ;
        if (c != '#')
            break;
        do
            c = getc(file);
        while ((c != '\n') && (c != EOF));
        if (c == EOF)
            break;
    }

    t = tok;

    if (c != EOF)
    {
        do
        {
            *t++ = c;
            c = getc(file);
        } while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

        if (c == '#')
            ungetc(c, file);
    }

    *t = 0;

    return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
    int x, y;
    int countbits;
    long int pos, counttotalbytes;
    unsigned char *p = databit;

    *p = 0;
    countbits = 1;
    counttotalbytes = 0;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = width * y + x;

            if (countbits <= 8)
            {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //*p |= (datauchar[pos] != 0) << (8 - countbits);

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                *p |= (datauchar[pos] == 0) << (8 - countbits);

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1))
            {
                p++;
                *p = 0;
                countbits = 1;
                counttotalbytes++;
            }
        }
    }

    return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
    int x, y;
    int countbits;
    long int pos;
    unsigned char *p = databit;

    countbits = 1;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = width * y + x;

            if (countbits <= 8)
            {
                // Numa imagem PBM:
                // 1 = Preto
                // 0 = Branco
                //datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

                // Na nossa imagem:
                // 1 = Branco
                // 0 = Preto
                datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

                countbits++;
            }
            if ((countbits > 8) || (x == width - 1))
            {
                p++;
                countbits = 1;
            }
        }
    }
}

IVC *vc_read_image(char *filename)
{
    FILE *file = NULL;
    IVC *image = NULL;
    unsigned char *tmp;
    char tok[20];
    long int size, sizeofbinarydata;
    int width, height, channels;
    int levels = 255;
    int v;

    // Abre o ficheiro
    if ((file = fopen(filename, "rb")) != NULL)
    {
        // Efectua a leitura do header
        netpbm_get_token(file, tok, sizeof(tok));

        if (strcmp(tok, "P4") == 0)
        {
            channels = 1;
            levels = 1;
        } // Se PBM (Binary [0,1])
        else if (strcmp(tok, "P5") == 0)
            channels = 1; // Se PGM (Gray [0,MAX(level,255)])
        else if (strcmp(tok, "P6") == 0)
            channels = 3; // Se PPM (RGB [0,MAX(level,255)])
        else
        {
#ifdef VC_DEBUG
            printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

            fclose(file);
            return NULL;
        }

        if (levels == 1) // PBM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca mem�ria para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL)
                return NULL;

            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char *)malloc(sizeofbinarydata);
            if (tmp == NULL)
                return 0;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                free(tmp);
                return NULL;
            }

            bit_to_unsigned_char(tmp, image->data, image->width, image->height);

            free(tmp);
        }
        else // PGM ou PPM
        {
            if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
                sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

                fclose(file);
                return NULL;
            }

            // Aloca mem�ria para imagem
            image = vc_image_new(width, height, channels, levels);
            if (image == NULL)
                return NULL;

#ifdef VC_DEBUG
            printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

            size = image->width * image->height * image->channels;

            if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
            {
#ifdef VC_DEBUG
                printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

                vc_image_free(image);
                fclose(file);
                return NULL;
            }
        }

        fclose(file);
    }
    else
    {
#ifdef VC_DEBUG
        printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
    }

    return image;
}

int vc_write_image(char *filename, IVC *image)
{
    FILE *file = NULL;
    unsigned char *tmp;
    long int totalbytes, sizeofbinarydata;

    if (image == NULL)
        return 0;

    if ((file = fopen(filename, "wb")) != NULL)
    {
        if (image->levels == 1)
        {
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
            tmp = (unsigned char *)malloc(sizeofbinarydata);
            if (tmp == NULL)
                return 0;

            fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

            totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
            printf("Total = %ld\n", totalbytes);
            if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
            {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                free(tmp);
                return 0;
            }

            free(tmp);
        }
        else
        {
            fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

            if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
            {
#ifdef VC_DEBUG
                fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

                fclose(file);
                return 0;
            }
        }

        fclose(file);

        return 1;
    }

    return 0;
}

/*
 * Função: vc_gray_to_binary_max_min
 * ----------------------------
 *   Faz a segmentação de uma imagem  com um threshold maximo e minimo
 *
 *   src:           imagem original 
 *   dst:           imagem de destino
 *   minThreshold:  threshold minimo
 *   maxThreshold:  threshold máximo
 */
int vc_gray_to_binary_max_min(IVC *src, IVC *dst, int minThreshold, int maxThreshold)
{
    unsigned char *datasrc = (unsigned char*)src->data;
    unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	long int pos;
	int size = width * height;

    //Verificação de erros
    if ((width <= 0) || (height <= 0) || (datasrc == NULL) || datadst == NULL) return 0;
	if (width != dst->width || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;

    
    for (int i = 0; i < size; i++)
    {
        if (src->data[i] > maxThreshold || src->data[i] < minThreshold)
        {
            dst->data[i] = 0;
        }
        else
        {
            dst->data[i] = 255;
        }
    }
}

/*
 * Função: vc_binary_circular_erode
 * ----------------------------
 *   Faz o processo de eliminar ruído indesejado de uma imagem
 *   utilizando um kernel circular
 *
 *   src:           imagem original 
 *   dst:           imagem de destino
 *   kernel:        kernel a utilizar(ímpar)
 */
int vc_binary_circular_erode(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, xAux, yAux;
	int offsetY = (kernel - 1) / 2, offsetX;
	long int pos, posk;
	int blackFound;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;
	if ((kernel <= 1) || (kernel % 2 == 0)) return 0; // Kernel tem que ser > 1 e ímpar

	// Percorrer os píxeis da imagem
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			// Posição do píxel a ser analisado neste momento
			pos = y * bytesperline + x; // * channels; (channels = 1)

			// Se o píxel que está a ser analisado for branco, quer dizer que pode ser alterado
			if ((int)datasrc[pos] != 0)
			{
				// Percorrer vizinhos c/ kernel circular (até encontrar um vizinho preto -> [blackFound])
				// Coordenadas de um vizinho = (xAux,yAux)
				for (yAux = y - offsetY, blackFound = 0; yAux <= y + offsetY && !blackFound; yAux++)
				{ // Calcular o offset em X (aumenta e depois diminui -> faz o "círculo")
					offsetX = offsetY - abs(yAux - y);
					for (xAux = x - offsetX; xAux <= x + offsetX && !blackFound; xAux++)
					{
						// Verificar se o vizinho está dentro da imagem
						if ((yAux >= 0) && (yAux < height) && (xAux >= 0) && (xAux < width))
						{
							// Posição do vizinho a ser analisado neste momento
							posk = yAux * bytesperline + xAux; // *channels; (channels = 1)
							
							// Se pelo menos um vizinho no kernel for preto
							if ((int)datasrc[posk] == 0) blackFound = 1;
						}
					}
				}

				// Se foi encontrado um vizinho preto, o píxel que acabou de ser analisado passa a ser preto
				if (blackFound) datadst[pos] = (unsigned char)0;
				else datadst[pos] = (unsigned char)255; // Se não, fica branco
			}
			else datadst[pos] = (unsigned char)0; // Se é preto, continua preto
		}
	}

	return 1;
}

/*
 * Função: vc_binary_circular_erode
 * ----------------------------
 *   Faz o processo de dilatação uma imagem
 *   utilizando um kernel circular
 *
 *   src:           imagem original 
 *   dst:           imagem de destino
 *   kernel:        kernel a utilizar(ímpar)
 */
int vc_binary_circular_dilate(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, xAux, yAux;
	int offsetY = (kernel - 1) / 2, offsetX;
	long int pos, posk;
	int whiteFound;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;
	if ((kernel <= 1) || (kernel % 2 == 0)) return 0; // Kernel tem que ser > 1 e ímpar

	// Percorrer os píxeis da imagem
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			// Posição do píxel a ser analisado neste momento
			pos = y * bytesperline + x; // * channels; (channels = 1)

			// Se o píxel que está a ser analisado for preto, quer dizer que pode ser alterado
			if ((int)datasrc[pos] == 0)
			{
				// Percorrer vizinhos c/ kernel circular (até encontrar um vizinho branco -> [whiteFound])
				// Coordenadas de um vizinho = (xAux,yAux)
				for (yAux = y - offsetY, whiteFound = 0; yAux <= y + offsetY && !whiteFound; yAux++)
				{ // Calcular o offset em X (aumenta e depois diminui -> faz o "círculo")
					offsetX = offsetY - abs(yAux - y);
					for (xAux = x - offsetX; xAux <= x + offsetX && !whiteFound; xAux++)
					{
						// Verificar se o vizinho está dentro da imagem
						if ((yAux >= 0) && (yAux < height) && (xAux >= 0) && (xAux < width))
						{
							// Posição do vizinho a ser analisado neste momento
							posk = yAux * bytesperline + xAux; // *channels; (channels = 1)

							// Se pelo menos um vizinho no kernel for branco
							if ((int)datasrc[posk] != 0) whiteFound = 1;
						}
					}
				}

				// Se foi encontrado um vizinho branco, o píxel que acabou de ser analisado passa a ser branco
				if (whiteFound) datadst[pos] = (unsigned char)255;
				else datadst[pos] = (unsigned char)0; // Se não, fica preto
			}
			else datadst[pos] = (unsigned char)255; // Se é branco, continua branco
		}
	}

	return 1;
}

/*
 * Função: vc_binary_open_circular
 * ----------------------------
 *   Faz a erosão seguida de uma dilatação de uma imagem
 *   utilizando um kernel circular
 *
 *   src:    imagem original 
 *   dst:    imagem de destino
 *   kernel: kernel a utilizar(ímpar)
 */
int vc_binary_open_circular(IVC *src, IVC *dst, int kernelErode, int kernelDilate)
{
	int output = 1;
    IVC* imageAux = vc_image_new(src->width, src->height, src->channels, src->levels);

    output &= vc_binary_circular_erode(src, imageAux, kernelErode);
    output &= vc_binary_circular_dilate(imageAux, dst, kernelDilate);

    vc_image_free(imageAux);

    return output;
}

/*
 * Função: vc_binary_close_circular
 * ----------------------------
 *   Faz a dilatação seguida de uma erosão de uma imagem
 *   utilizando um kernel quadrado
 *
 *   src:    imagem original 
 *   dst:    imagem de destino
 *   kernel: kernel a utilizar(ímpar)
 */
int vc_binary_close_circular(IVC *src, IVC *dst, int kernelErode, int kernelDilate)
{
    IVC *tmp = vc_image_new(src->width, src->height, src->channels, src->levels);

    vc_binary_circular_dilate(src, tmp, kernelDilate);
    vc_binary_circular_erode(tmp, dst, kernelErode);
}

/*
 * Função: vc_binary_blob_labelling
 * ----------------------------
 *   Faz a etiquetagem de blobs numa imagem
 *   binária
 *
 *   src		: Imagem binária de entrada 
 *   dst		: Imagem grayscale (irá conter as etiquetas)
 *   nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas
 *
 *   Retorna    : um array de estruturas de blobs (objectos), com respectivas etiquetas.
 */
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

		/* 	posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X */

			posA = (int) datadst[(y - 1) * bytesperline + (x - 1) * channels];
            posB = (int) datadst[(y - 1) * bytesperline + x * channels];
            posC = (int) datadst[(y - 1) * bytesperline + (x + 1) * channels];
            posD = (int) datadst[y * bytesperline + (x - 1) * channels];
            posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;
   
	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

/*
 * Função: vc_binary_blob_info
 * ----------------------------
 *   Calcula a área, o perimetro e o centro de massa de blobs de uma imagem
 *
 *   src		: Imagem binária de entrada 
 *   blobs		: Estrutura que armazena a informação calculada
 *   nblobs  	: Número de blobs na imagem
 */
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

    

	// Conta área de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;
	
		// Centro de Gravidade
        // x médio (média de todos os valores/coordenadas em x)
        blobs[i].xc = round((float)sumx / (float)MAX(blobs[i].area, 1)); // Usa-se o MAX para nunca dar 0
        // y médio (média de todos os valores/coordenadas em y)
        blobs[i].yc = round((float)sumy / (float)MAX(blobs[i].area, 1));
	}

	return 1;
}

/*
 * Função: vc_binary_to_gray
 * ----------------------------
 *   Para os pixeis a branco de uma imagem binária, vai buscar o valor do mesmo pixel
 *   na imagem a cinzento e implementa esse valor no pixel da imagem de destino
 *
 *   original	: Imagem original 
 *   binary		: Imagem binária
 *   dst    	: Imagem de destino
 */
int vc_binary_to_gray(IVC *original, IVC *binary, IVC *dst)
{
    //Verificação de erros
    if ((original->width <= 0) || (original->height <= 0) || (original->data == NULL) || (binary->data == NULL)) return 0;
	if ((original->width != binary->width) || (original->width != dst->width) || (original->height != binary->height) || 
        (original->height != dst->height) || (original->channels != binary->channels) || (original->channels != dst->channels)) return 0;
	if (original->channels != 1) return 0;

    int size = original->width * original->height;

    for(int i = 0; i < size; i++)
    {
        if(binary->data[i] != 0)
        {
            dst->data[i] = original->data[i];
        }
    }
}

/*
 * Função: vc_rgb_get_blue_gray

 * ----------------------------
 *   Extrai a componente azul de uma imagem RGB para uma
 *   imagem em tons de cinzento
 *
 *   src    	: Imagem RGB
 *   dst    	: Imagem de destino
 */
int vc_rgb_get_blue_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	int bytesperlinesrc = width * channels;
	int bytesperlinedst = width * dst->channels;
	int x, y;
	long int posSrc, posDst;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height)) return 0;
	if (channels != 3 || dst->channels != 1) return 0;

	// Extrai a componente Blue
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			posSrc = y * bytesperlinesrc + x * channels;
			posDst = y * bytesperlinedst + x; // * channels (= 1)

			datadst[posDst] = (unsigned char) datasrc[posSrc + 2]; // Igualar ao valor de B (Blue/Azul)
		}
	}

	return 1;
}

/*
 * Função: vc_gray_to_binary

 * ----------------------------
 *   Segmentação de uma imagem em tons de cinzento para 
 *   uma imagem binária
 *
 *   src    	: Imagem em tons de cinzento
 *   dst    	: Imagem binária de destino
 *   threshold  : Threshold máximo
 */
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = src->channels;
	long int pos;
	int size = width * height;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || datadst == NULL) return 0;
	if (width != dst->width || (height != dst->height) || (channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	// Percorre píxeis (ponto a ponto)
	for (pos = 0; pos < size; pos++)
	{ // De acordo com o treshold, a cor será preta ou branca
		if ((int) datasrc[pos] < threshold) datadst[pos] = (unsigned char) 0;
		else datadst[pos] = (unsigned char) 255;
	}

	return 1;
}

/*
 * Função: vc_mark_blobs

 * ----------------------------
 *   Faz a marcação do centro de massa e a caixa delimitadora
 *   de cada blob numa nova imagem.
 *   Definido apenas para imagens RGB.
 *
 *   src    	: Imagen RGB
 *   dst        : Imagem de destino
 *   blobs    	: Estrutura que guarda informação das blobs
 *   nblobs     : Número de blobs na imagem
 */
int vc_mark_blobs(IVC* src, IVC* dst, OVC* blobs, int nblobs)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = dst->width;
	int height = dst->height;
	int channels = dst->channels;
	int bytesperline = src->bytesperline;
	long int pos;
	int x, y, i, blobXmin, blobXmax, blobYmin, blobYmax;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (datadst == NULL)) return 0;
	if ((width != dst->width) || (height != dst->height) || (channels != dst->channels)) return 0;
	if ((blobs == NULL) || (nblobs <= 0)) return 0;
	if (channels != 3) return 0;

	// Copia dados da imagem original para a nova imagem
	memcpy(datadst, datasrc, bytesperline * height);

	// Percorre cada blob
	for (i = 0; i < nblobs; i++)
	{
		// Marcar o centro de massa
		pos = blobs[i].yc * bytesperline + blobs[i].xc * channels;
		datadst[pos] = (unsigned char) 127;
		datadst[pos + 1] = (unsigned char) 127;
		datadst[pos + 2] = (unsigned char) 127;
		datadst[pos - 1] = (unsigned char) 255;
		datadst[pos - 2] = (unsigned char) 255;
		datadst[pos - 3] = (unsigned char) 255;
		datadst[pos + 3] = (unsigned char) 255;
		datadst[pos + 4] = (unsigned char) 255;
		datadst[pos + 5] = (unsigned char) 255;
		datadst[pos - bytesperline] = (unsigned char) 255;
		datadst[pos - bytesperline + 1] = (unsigned char) 255;
		datadst[pos - bytesperline + 2] = (unsigned char) 255;
		datadst[pos + bytesperline] = (unsigned char) 255;
		datadst[pos + bytesperline + 1] = (unsigned char) 255;
		datadst[pos + bytesperline + 2] = (unsigned char) 255;


		// Coordenadas da caixa delimitadora
		blobYmin = blobs[i].y;
		blobYmax = blobYmin + blobs[i].height - 1;
		blobXmin = blobs[i].x;
		blobXmax = blobXmin + blobs[i].width - 1;

		// Marcar a caixa delimitadora
		// Limites verticais
		for (y = blobYmin; y <= blobYmax; y++)
		{
			// Limite esquerdo
			pos = y * bytesperline + blobXmin * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;

			// Limite direito
			pos = y * bytesperline + blobXmax * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;
		}
		// Limites horizontais
		for (x = blobXmin; x <= blobXmax; x++)
		{
			// Limite superior
			pos = blobYmin * bytesperline + x * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;

			// Limite inferior
			pos = blobYmax * bytesperline + x * channels;
			datadst[pos] = (unsigned char)255;
			datadst[pos + 1] = (unsigned char)255;
			datadst[pos + 2] = (unsigned char)255;
		}
	}

	return 1;
}