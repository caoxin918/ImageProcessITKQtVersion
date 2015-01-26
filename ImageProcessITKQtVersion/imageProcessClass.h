#ifndef IMAGEPROCESSCLASS_H
#define IMAGEPROCESSCLASS_H
#include <string>

#include "QImage"
#include "QString"
#include "QThread"
#include "QColor"
#include "QRgb"
#include "QDir"
#include <QMessageBox>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkTIFFImageIO.h"
#include "itkPNGImageIO.h"
#include "itkMedianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkScalarToRGBColormapImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkSimilarity2DTransform.h"
#include "itkScaleTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

typedef unsigned short PixelType;
const unsigned int Dimension=2;
typedef itk::Image<PixelType,Dimension> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType;//��������Լ������ӫ��ͼ���ʽ������16bit tif�Ҷ�ͼ��
typedef unsigned char UnChPixelType;//����İ׹�ͼ��Ϊ8bit
typedef itk::Image<UnChPixelType,Dimension> UnChImageType;//�׹�ͼ���ITK����
typedef itk::RescaleIntensityImageFilter<ImageType,ImageType> RescaleFilterType;
typedef itk::CastImageFilter<ImageType,UnChImageType> CastFilterType;
typedef itk::RGBPixel< unsigned char >        RGBPixelType;
typedef itk::Image< RGBPixelType, Dimension > RGBImageType;//α��ɫͼ���ITK����
typedef itk::ImageFileWriter<RGBImageType> RGBWriterType;//���8bit rgbͼ��
typedef itk::ImageFileReader<RGBImageType> RGBReaderType;
typedef itk::ImageFileWriter<UnChImageType> UnChWriterType;//���8bitͼ��
typedef itk::ImageFileReader<UnChImageType> UnChReaderType;
typedef itk::ImageRegionIterator<RGBImageType> RGBIteratortype;
typedef itk::ImageRegionIterator<UnChImageType> UnChIteratorType;
typedef itk::ScalarToRGBColormapImageFilter< UnChImageType, RGBImageType> RGBFilterType;
typedef itk::RGBAPixel<unsigned char> RGBAPixelType;//��������pngͼƬ�����ظ�ʽ
typedef itk::Image< RGBAPixelType, Dimension > RGBAImageType;
typedef itk::ImageRegionIterator<RGBAImageType> RGBAIteratortype;
typedef itk::ImageFileWriter<RGBAImageType> RGBAWriterType;
typedef itk::ImageRegionConstIterator<ImageType> ConstIteratorType;
typedef itk::ImageRegionIterator<ImageType> IteratorType;//ͼ�������
typedef itk::TIFFImageIO TIFFIOType;//IO�޶�Ϊtiff��ʽͼ��
typedef itk::PNGImageIO PNGIOType;
typedef itk::MedianImageFilter<ImageType,ImageType> MedianFilterType;//ͼ���˲���

using namespace std;
using namespace cv;
/****************************************������ͼ������******************************************************/
class ImageProcessClass
{
public:
	bool readLuminescence(QString filename);
	bool readPhotograph(QString filename);
	void substract(ImageType::PixelType backgroundValue);
	void denoise(unsigned int kernerSize);
	void pseudocolor(int colormapType,bool hasPhotoraph,int highValue,int lowValue);//define the jet 0;
	void writeImage(bool saveAllImages,QString filename);
	void setResizeStrategy(bool resizeFlag);
	bool isLuminescenceLargerThanPhotograph();
	bool isDenoiseLargeEqualToResizedPhotograph();

	void saveSingleImage(QString filename,ImageType::Pointer inputImage);
	void saveSingleImage(QString filename,RGBImageType::Pointer inputImage);
	void saveSingleImage(QString filename,RGBAImageType::Pointer inputImage);
	void saveSingleImage(QString filename,UnChImageType::Pointer inputImage);

	void initial();
	QString getWarningMessage(){return warningMessage;clearWarningMessage();}
	void clearWarningMessage(){warningMessage="";}

	QImage* getLuminescenceLabelQImage();
	QImage* getPhotographLabelQImage();
	QImage* getFusionLabelQImage();


// 	ImageType::Pointer readLuminescenceImage(QString filename);//read a luminescence image file using ITK
// 	ImageType::Pointer readPhotographImage(QString filename);//read a photograph image file using ITK

	void ITK2QImage(ImageType::Pointer itkData,QImage* labelQImage);//convert an gray-scale ITK data to QImage data
	void ITK2QImage(UnChImageType::Pointer itkData,QImage* labelQImage);
	void ITK2QImage(RGBImageType::Pointer itkData,QImage* labelQImage);//convert an RGB-scaleITK data to QImage data

	void substractBackground(ImageType::Pointer inputImage, ImageType::PixelType backgroundValue);
	void substractBackground(UnChImageType::Pointer inputImage, UnChImageType::PixelType backgroundValue);
	void copyImageData(ImageType::Pointer inputData,ImageType::Pointer outputData);
	void copyImageData(RGBImageType::Pointer inputData,RGBImageType::Pointer outputData);

	ImageType::Pointer Mat2ITK(Mat inputImg,ImageType::Pointer outputImg);//resize the image using OpenCV


	void fusion();


	UnChImageType::Pointer rescaleImage(ImageType::Pointer imageData,PixelType minValue,PixelType maxValue);
	void sliceInputLuminescneceImage(ImageType::Pointer inputImage,PixelType HValue,PixelType LValue);
	PixelType maxValue(ImageType::Pointer inputImg);
	PixelType minValue(ImageType::Pointer inputImg);
	PixelType getHighValue(){return pseudocolorHighValue;}
	PixelType getLowValue(){return pseudocolorLowValue;}


/*private:*/
	ImageProcessClass();
	~ImageProcessClass();
private:
	ReaderType::Pointer readerForGrayscale;
	WriterType::Pointer writerForGrayscale;
	RGBWriterType::Pointer writerForRGB;
	RGBAWriterType::Pointer writerForRGBA;
	TIFFIOType::Pointer tiffIO;
	QImage* luminescenceLabelQImage;
	QImage* photographLabelQImage;
	QImage* fusionLabelQImage;
	ImageType::Pointer originalLuminesence;
	ImageType::Pointer originalPhotograph;
	ImageType::Pointer resizedLuminesence;
	ImageType::Pointer resizedPhotograph;
	ImageType::Pointer denoiseLuminescence;
	ImageType::Pointer substractedLuminescence;
	UnChImageType::Pointer sliced8BitsLuminescence;
	UnChImageType::Pointer rescaledPhotograph;
	RGBImageType::Pointer pseudocolorLuminescence;
	RGBAImageType::Pointer pseudocolorLuminescenceWithAlpha;
	RGBImageType::Pointer fusionImage;
	PixelType pseudocolorHighValue,pseudocolorLowValue;

	bool resizeFlag;//true means resize to luminescence, while false means resize to the large image
	Mat originalLuminesenceMat;
	Mat originalPhotographMat;
	int kernerSize;
	QString warningMessage;


};



#endif