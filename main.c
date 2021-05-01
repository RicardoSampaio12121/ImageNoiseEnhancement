/*
Author: Filipe Gajo
Author: Ricardo Sampaio
Author: Claudio Silva
*/

#include <stdio.h>
#include <stdlib.h>
#include "vc.h"


int mainP1(void) //P1
{
    IVC *imagemCerebro, *imagemSegmentada, *imagemOpen, *imagemClose, *imagemLabelled, *imagemFinal;
    OVC *blobs;

    int nlabels;
	
    imagemCerebro = vc_read_image("Imagens/cerebro.pgm");
    imagemSegmentada = vc_image_new(imagemCerebro->width, imagemCerebro->height, imagemCerebro->channels, imagemCerebro->levels);
    imagemOpen = vc_image_new(imagemCerebro->width, imagemCerebro->height, imagemCerebro->channels, imagemCerebro->levels); 
    imagemClose = vc_image_new(imagemCerebro->width, imagemCerebro->height, imagemCerebro->channels, imagemCerebro->levels); 
    imagemLabelled = vc_image_new(imagemCerebro->width, imagemCerebro->height, imagemCerebro->channels, imagemCerebro->levels); 
    imagemFinal = vc_image_new(imagemCerebro->width, imagemCerebro->height, imagemCerebro->channels, imagemCerebro->levels); 

    if(imagemCerebro == NULL)
    {
        printf("Error -> vc_read_image(): \n\tFile not found!\n");
        getchar();
        return 0;
    }

    vc_gray_to_binary_max_min(imagemCerebro, imagemSegmentada, 74, 199);    // Faz a segmentação da imagem original
    vc_binary_open_circular(imagemSegmentada, imagemOpen, 9, 9);         // Exclui o restante do cranio que sobrou da segmentação
    vc_binary_close_circular(imagemOpen, imagemClose, 9, 9);        // Fecha buracos no cerebro resultantes do open

    blobs = vc_binary_blob_labelling(imagemClose, imagemLabelled, &nlabels); // Faz o labbeling da imagem

    if (blobs != NULL)
    {
        vc_binary_blob_info(imagemLabelled, blobs, nlabels); // Faz o calculo de algumas informações da imagem
    }

    vc_binary_to_gray(imagemCerebro, imagemClose, imagemFinal); // Coloriza os pixeis da imagem com os respetivos pixeis da imagem original
    
    // Mostrar info da imagem
    printf("Área: %d\n"                 , blobs->area);
    printf("Perimetro: %d\n"            , blobs->perimeter);
    printf("Centro de massa: (%d, %d)\n", blobs->xc, blobs->yc);
    
    // Escreve as imagens
    vc_write_image("Resultados/P1/imagemBinarizacao.pgm" , imagemSegmentada);
    vc_write_image("Resultados/P1/imagemAbertura.pgm"    , imagemOpen);
    vc_write_image("Resultados/P1/imagemFechamento.pgm"  , imagemClose);
    vc_write_image("Resultados/P1/imagemFinal.pgm"       , imagemFinal);

    // Liberta as imagens da memória  
    vc_image_free(imagemCerebro);
    vc_image_free(imagemSegmentada);
    vc_image_free(imagemOpen);
    vc_image_free(imagemLabelled);
    vc_image_free(imagemClose);
    vc_image_free(imagemFinal);

    //Abre imagens
    system("cmd /c start FilterGear Imagens/cerebro.pgm");                  // Input
	system("cmd /c start FilterGear Resultados/P1/imagemBinarizacao.pgm");  // Output
	system("cmd /c start FilterGear Resultados/P1/imagemAbertura.pgm");     // Output
	system("cmd /c start FilterGear Resultados/P1/imagemFechamento.pgm");   // Output
	system("cmd /c start FilterGear Resultados/P1/imagemFinal.pgm");        // Output

	printf("Pressione qualquer tecla para sair...\n");
	getchar();
    
    return 0;
}

int main(void) //P2
{
	IVC* imagemCelulas, * imagemCinzentos, * imagemBinarizacao, * imagemAbertura, * imagemFechamento, * imagemLabels, * imagemMarcada;
	OVC* blobs;
	int nblobs;

	// Carregar imagem das células do epitélio pigmentar da retina (EPR).
	imagemCelulas = vc_read_image("Imagens/celulas.ppm"); // 400x397

	// Verificação de erros
	if (imagemCelulas == NULL)
	{
		printf("ERROR -> vc_read_image():\n\tFicheiro nao encontrado!\n");
		getchar();
		return 0;
	}

	// Criação das imagens para guardar as várias operações
	imagemCinzentos = vc_image_new(imagemCelulas->width, imagemCelulas->height, 1, imagemCelulas->levels);
	imagemBinarizacao = vc_image_new(imagemCelulas->width, imagemCelulas->height, 1, imagemCelulas->levels);
	imagemAbertura = vc_image_new(imagemCelulas->width, imagemCelulas->height, 1, imagemCelulas->levels);
	imagemFechamento = vc_image_new(imagemCelulas->width, imagemCelulas->height, 1, imagemCelulas->levels);
	imagemLabels = vc_image_new(imagemCelulas->width, imagemCelulas->height, 1, imagemCelulas->levels);
	imagemMarcada = vc_image_new(imagemCelulas->width, imagemCelulas->height, imagemCelulas->channels, imagemCelulas->levels);

	// Calcular o nº de células
	// 1 - Transformar a imagem a cores numa imagem em tons de cinzento (só a partir do azul)
	// Extrair a componente azul da imagem a cores numa imagem em tons de cinzento
	vc_rgb_get_blue_gray(imagemCelulas, imagemCinzentos);
	vc_write_image("Resultados/P2/imagemCinzentos.pgm", imagemCinzentos);

    // 2 - Transformar imagem em tons de cinzento numa imagem binária
	// Threshold calculado a partir de várias medições da cor das células no Gimp (33)
	vc_gray_to_binary(imagemCinzentos, imagemBinarizacao, 33);
	vc_write_image("Resultados/P2/imagemBinarizacao.pgm", imagemBinarizacao);
	
    // 3 - Eliminar ruído da imagem binária (c/ abertura)
	vc_binary_open_circular(imagemBinarizacao, imagemAbertura, 7, 7);
	vc_write_image("Resultados/P2/imagemAbertura.pgm", imagemAbertura);
	
    // 4 - Preencher falhas dentro das células (c/ fechamento)
	vc_binary_close_circular(imagemAbertura, imagemFechamento, 11, 11);
	vc_write_image("Resultados/P2/imagemFechamento.pgm", imagemFechamento);
	puts("chega aqui");
	
    // 5 - Calcular o nº de células
	blobs = vc_binary_blob_labelling(imagemFechamento, imagemLabels, &nblobs);
	printf("\nNumero de celulas: %d\n\n", nblobs);
	puts("chega aqui 2");

	// Calcular a área de cada núcleo
	vc_binary_blob_info(imagemLabels, blobs, nblobs);
	if (blobs != NULL)
	{
		for (int i = 0; i < nblobs; i++)
		{
			printf("-> Etiqueta %d\n", blobs[i].label);
			printf("-> Area %d\n\n", blobs[i].area);
		}
	}

	// Criar imagem de saída com centro de massa e caixa delimitadora de cada núcleo identificado
	vc_mark_blobs(imagemCelulas, imagemMarcada, blobs, nblobs);
	vc_write_image("Resultados/P2/imagemMarcada.ppm", imagemMarcada);

	free(blobs);

	// Libertar as imagens da memória
	vc_image_free(imagemCelulas);
	vc_image_free(imagemCinzentos);
	vc_image_free(imagemBinarizacao);
	vc_image_free(imagemAbertura);
	vc_image_free(imagemFechamento);
	vc_image_free(imagemLabels);
	vc_image_free(imagemMarcada);

	// Mostrar imagem carregada e imagens escritas
	system("cmd /c start FilterGear Imagens/celulas.ppm"); // Input
	system("cmd /c start FilterGear Resultados/P2/imagemCinzentos.pgm"); // Output
	system("cmd /c start FilterGear Resultados/P2/imagemBinarizacao.pgm"); // Output
	system("cmd /c start FilterGear Resultados/P2/imagemAbertura.pgm"); // Output
	system("cmd /c start FilterGear Resultados/P2/imagemFechamento.pgm"); // Output
	system("cmd /c start FilterGear Resultados/P2/imagemMarcada.ppm"); // Output

	printf("Pressione qualquer tecla para sair...\n");

	getchar();
	return 0;
} 