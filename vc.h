//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)


typedef struct {
    unsigned char *data;
    int width, height;
    int channels;			// Bin�rio/Cinzentos=1; RGB=3
    int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
    int bytesperline;		// width * channels
} IVC;


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

//Segmentação de imagem
int vc_gray_to_binary_max_min(IVC *src, IVC *dst, int minThreshold, int maxThreshold);
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);

//Operadores morfológicos
int vc_binary_circular_erode(IVC* src, IVC* dst, int kernel);
int vc_binary_circular_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_open_circular(IVC *src, IVC *dst, int kernelErode, int kernelDilate);
int vc_binary_close_circular(IVC *src, IVC *dst, int kernelErode, int kernelDilate);

//Blobs e etiquetagem
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);
int vc_mark_blobs(IVC* src, IVC* dst, OVC* blobs, int nblobs);

//Extração de cores de imagens RGB
int vc_rgb_get_blue_gray(IVC* src, IVC* dst);

//Colorização
int vc_binary_to_gray(IVC *original, IVC *binary, IVC *dst);