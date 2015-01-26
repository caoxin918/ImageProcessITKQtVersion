#include "imageProcessClass.h"

ImageProcessClass::ImageProcessClass()
{
	initial();
}
ImageProcessClass::~ImageProcessClass()
{

}
bool ImageProcessClass::readLuminescence(QString filename)
{
	initial();
	if(filename.isEmpty())
	{
		warningMessage="Canceled!\n";
		return false;
	}
	string temp1=filename.toStdString();
	const char* temp2=temp1.c_str();
	originalLuminesenceMat=imread(temp2,CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH);
	if (originalLuminesenceMat.channels()!=1)
	{
		warningMessage="Input photograph must be gray-scaled! No RGB Image!\n";
		return false;//input image must be one channel tif image
	}
	Mat2ITK(originalLuminesenceMat,originalLuminesence);
	int rows=originalLuminesence->GetRequestedRegion().GetSize()[0];
	int cols=originalLuminesence->GetRequestedRegion().GetSize()[1];
	resizedLuminesence=originalLuminesence;
	luminescenceLabelQImage=new QImage(rows,cols,QImage::Format_RGB888);//here to define the luminescenceLabelQImage
	ITK2QImage(resizedLuminesence,luminescenceLabelQImage);
	warningMessage=QString("Luminescence loaded!\n")+QString("FileName: ")+QString(filename)+QString("\n")+QString("Size: ")+QString::number(rows)+QString("*")+QString::number(cols)+QString("\n");
	
	return true;
}
// ImageType::Pointer ImageProcessClass::readLuminescenceImage(QString filename)
// {
// 	string temp1=filename.toStdString();
// 	const char* temp2=temp1.c_str();
// 	readerForGrayscale->SetFileName(temp2);
// 	readerForGrayscale->SetImageIO(tiffIO);
// 	readerForGrayscale->Update();
// 	return readerForGrayscale->GetOutput();
// }
bool ImageProcessClass::readPhotograph(QString filename)
{
	
	originalPhotograph=NULL;
	originalPhotograph=ImageType::New();

	resizedPhotograph=NULL;
	resizedPhotograph=ImageType::New();
	
	if(filename.isEmpty())
	{
		warningMessage="Canceled!\n";
		return false;
	}
	originalPhotographMat=NULL;
	string temp1=filename.toStdString();
	const char* temp2=temp1.c_str();
	originalPhotographMat=imread(temp2,CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH);
	if(originalPhotographMat.channels()!=1)
	{
		warningMessage="Input photograph must be gray-scaled! No RGB Image!\n";
		return false;
	}
	if((double)originalPhotographMat.rows/originalPhotographMat.cols != (double)originalLuminesenceMat.rows/originalLuminesenceMat.cols)
	{
		warningMessage="The length-width ratio of input photograph is not the same with input luminescence!\n";
		return false;//luminescence must have the same ratio compared to photograph
	}
	warningMessage=QString("Photograph loaded!\n")+QString("FileName: ")+QString(filename)+QString("\n")+QString("Size: ")+QString::number(originalPhotographMat.rows)+QString("*")+QString::number(originalPhotographMat.cols)+QString("\n");

	Mat2ITK(originalPhotographMat,originalPhotograph);
	resizedLuminesence=NULL;
	resizedLuminesence=ImageType::New();
	Mat resizedPhotographMat;
	Mat resizedLuminesenceMat;
	if(resizeFlag)//resize to luminescence
	{
		resizedLuminesence=NULL;
		resizedLuminesence=ImageType::New();
		resizedLuminesence=originalLuminesence;
		if (originalPhotographMat.rows==originalLuminesenceMat.rows)
		{
			resizedPhotograph=originalPhotograph;
		}
		else
		{
			cv::resize(originalPhotographMat,resizedPhotographMat,Size(originalLuminesenceMat.rows,originalLuminesenceMat.cols));
			Mat2ITK(resizedPhotographMat,resizedPhotograph);
			warningMessage=warningMessage+QString("The size of photograph is resized to that of luminescence\n");
		}
	}
	else
	{
		if (originalPhotographMat.rows<originalLuminesenceMat.rows)
		{
			cv::resize(originalPhotographMat,resizedPhotographMat,Size(originalLuminesenceMat.rows,originalLuminesenceMat.cols));
			Mat2ITK(resizedPhotographMat,resizedPhotograph);
			resizedLuminesence=NULL;
			resizedLuminesence=ImageType::New();
			resizedLuminesence=originalLuminesence;
			warningMessage=warningMessage+QString("The size of photograph is resized to that of luminescence\n");
		} 
		else if(originalPhotographMat.rows>originalLuminesenceMat.rows)
		{
			cv::resize(originalLuminesenceMat,resizedLuminesenceMat,Size(originalPhotographMat.rows,originalPhotographMat.cols));
			Mat2ITK(resizedLuminesenceMat,resizedLuminesence);

			resizedPhotograph=originalPhotograph;

			denoiseLuminescence=NULL;
			denoiseLuminescence=ImageType::New();
			substractedLuminescence=NULL;
			substractedLuminescence=ImageType::New();
			sliced8BitsLuminescence=NULL;
			sliced8BitsLuminescence=UnChImageType::New();
			rescaledPhotograph=NULL;
			rescaledPhotograph=UnChImageType::New();
			pseudocolorLuminescence=NULL;
			pseudocolorLuminescence=RGBImageType::New();
			pseudocolorLuminescenceWithAlpha=NULL;
			pseudocolorLuminescenceWithAlpha=RGBAImageType::New();
			fusionImage=NULL;
			fusionImage=RGBImageType::New();
			warningMessage=warningMessage+QString("The size of luminescence is resized to that of photograph\n");
		}
		else
		{
			resizedLuminesence=originalLuminesence;
			resizedPhotograph=originalPhotograph;
		}
		if(denoiseLuminescence->GetRequestedRegion().GetSize()[0]!=0)
		{
			if (resizedLuminesence->GetRequestedRegion().GetSize()[0]!=denoiseLuminescence->GetRequestedRegion().GetSize()[0])
			{
				denoiseLuminescence=NULL;
				denoiseLuminescence=ImageType::New();
				denoise(kernerSize);
			}
		}
		
	}

	
	photographLabelQImage=new QImage(resizedPhotograph->GetRequestedRegion().GetSize()[0],resizedPhotograph->GetRequestedRegion().GetSize()[1],QImage::Format_RGB888);
	rescaledPhotograph=NULL;
	rescaledPhotograph=UnChImageType::New();
	rescaledPhotograph=rescaleImage(resizedPhotograph,0,255);
	ITK2QImage(rescaledPhotograph,photographLabelQImage);

	return true;
}
// ImageType::Pointer ImageProcessClass::readPhotographImage(QString filename)
// {
// 	string temp1=filename.toStdString();
// 	const char* temp2=temp1.c_str();
// 	readerForGrayscale=ReaderType::New();
// 	readerForGrayscale->SetFileName(temp2);
// 	readerForGrayscale->SetImageIO(tiffIO);
// 	readerForGrayscale->Update();
// 	return readerForGrayscale->GetOutput();
// }


UnChImageType::Pointer ImageProcessClass::rescaleImage(ImageType::Pointer imageData,PixelType minValue,PixelType maxValue)
{
	RescaleFilterType::Pointer rescaleFilter=RescaleFilterType::New();
	rescaleFilter->SetInput(imageData);
	rescaleFilter->SetOutputMinimum(minValue);
	rescaleFilter->SetOutputMaximum(maxValue);
	rescaleFilter->Update();
	CastFilterType::Pointer castFilter=CastFilterType::New();
	castFilter->SetInput(rescaleFilter->GetOutput());
	castFilter->Update();
	return castFilter->GetOutput();
}
void ImageProcessClass::ITK2QImage(ImageType::Pointer itkData,QImage* labelQImage)
{
 	int rows=itkData->GetRequestedRegion().GetSize()[0];
 	int cols=itkData->GetRequestedRegion().GetSize()[1];
	UnChImageType::Pointer temp8BitsImage=UnChImageType::New();
	temp8BitsImage=rescaleImage(itkData,0,255);
	UnChIteratorType it(temp8BitsImage,temp8BitsImage->GetRequestedRegion());
	it.GoToBegin();
	for(int i=0;i<rows;++i)
		for(int j=0;j<cols;++j)
		{
			labelQImage->setPixel(j,i,qRgb(it.Get(),it.Get(),it.Get()));
			++it;
		}
}
void ImageProcessClass::ITK2QImage(UnChImageType::Pointer itkData,QImage* labelQImage)
{
	int rows=itkData->GetRequestedRegion().GetSize()[0];
	int cols=itkData->GetRequestedRegion().GetSize()[1];
	UnChIteratorType it(rescaledPhotograph,rescaledPhotograph->GetRequestedRegion());
	it.GoToBegin();
	for(int i=0;i<rows;++i)
		for(int j=0;j<cols;++j)
		{
			labelQImage->setPixel(j,i,qRgb(it.Get(),it.Get(),it.Get()));
			++it;
		}
}
void ImageProcessClass::ITK2QImage(RGBImageType::Pointer itkData,QImage* labelQImage)
{
	int rows=itkData->GetRequestedRegion().GetSize()[0];
	int cols=itkData->GetRequestedRegion().GetSize()[1];
	RGBIteratortype it(itkData,itkData->GetRequestedRegion());
	it.GoToBegin();
	RGBPixelType temp;
	for(int i=0;i<rows;++i)
		for(int j=0;j<cols;++j)
		{
			temp=it.Get();
			labelQImage->setPixel(j,i,qRgb(temp.GetRed(),temp.GetGreen(),temp.GetBlue()));
			++it;
		}
}

void ImageProcessClass::initial()
{
	readerForGrayscale=ReaderType::New();
	writerForGrayscale=WriterType::New();
	writerForRGB=RGBWriterType::New();
	writerForRGBA=RGBAWriterType::New();
	tiffIO=TIFFIOType::New();

	luminescenceLabelQImage=new QImage();
	photographLabelQImage=new QImage();
	fusionLabelQImage=new QImage();
	originalLuminesence=NULL;
	originalLuminesence=ImageType::New();
	originalPhotograph=NULL;
	originalPhotograph=ImageType::New();
	denoiseLuminescence=NULL;
	denoiseLuminescence=ImageType::New();
	substractedLuminescence=NULL;
	substractedLuminescence=ImageType::New();
	sliced8BitsLuminescence=NULL;
	sliced8BitsLuminescence=UnChImageType::New();
	rescaledPhotograph=NULL;
	rescaledPhotograph=UnChImageType::New();
	pseudocolorLuminescence=NULL;
	pseudocolorLuminescence=RGBImageType::New();
	pseudocolorLuminescenceWithAlpha=NULL;
	pseudocolorLuminescenceWithAlpha=RGBAImageType::New();
	fusionImage=NULL;
	fusionImage=RGBImageType::New();
	resizedPhotograph=NULL;
	resizedPhotograph=ImageType::New();
	resizedLuminesence=NULL;
	resizedLuminesence=ImageType::New();

	resizeFlag=true;
	originalLuminesenceMat=NULL;
	originalPhotographMat=NULL;
	kernerSize=0;
}
QImage* ImageProcessClass::getLuminescenceLabelQImage()
{
	return luminescenceLabelQImage;
}
QImage* ImageProcessClass::getPhotographLabelQImage()
{
	return photographLabelQImage;
}
QImage* ImageProcessClass::getFusionLabelQImage()
{
	return fusionLabelQImage;
}
void ImageProcessClass::substract(ImageType::PixelType backgroundValue)
{
	substractedLuminescence=NULL;
	substractedLuminescence=ImageType::New();
	copyImageData(resizedLuminesence,substractedLuminescence);
	if(backgroundValue==0)
		return;
	substractBackground(substractedLuminescence,backgroundValue);


	luminescenceLabelQImage=NULL;
	int rows=substractedLuminescence->GetRequestedRegion().GetSize()[0];
	int cols=substractedLuminescence->GetRequestedRegion().GetSize()[1];
	luminescenceLabelQImage=new QImage(rows,cols,QImage::Format_RGB888);
	ITK2QImage(substractedLuminescence,luminescenceLabelQImage);

	
}
void ImageProcessClass::substractBackground(ImageType::Pointer inputImage, ImageType::PixelType backgroundValue)
{
	IteratorType it(inputImage,inputImage->GetRequestedRegion());
	it.GoToBegin();
	ImageType::PixelType temp=0;
	while(!it.IsAtEnd())
	{
		if(it.Get()>=backgroundValue)
			it.Set(it.Get()-backgroundValue);
		else
			it.Set(0);
		++it;
	}
}
void ImageProcessClass::copyImageData(ImageType::Pointer inputData,ImageType::Pointer outputData)
{
	outputData->SetRegions(inputData->GetRequestedRegion());
	outputData->CopyInformation(inputData);
	outputData->Allocate();

	IteratorType outputIt(outputData,outputData->GetRequestedRegion());
	ConstIteratorType inputIt(inputData,inputData->GetRequestedRegion());
	while(!inputIt.IsAtEnd())
	{
		outputIt.Set(inputIt.Get());
		++inputIt;
		++outputIt;
	}
}
void ImageProcessClass::copyImageData(RGBImageType::Pointer inputData,RGBImageType::Pointer outputData)
{
	outputData->SetRegions(inputData->GetRequestedRegion());
	outputData->CopyInformation(inputData);
	outputData->Allocate();
	RGBIteratortype outputIt(outputData,outputData->GetRequestedRegion());
	RGBIteratortype inputIt(inputData,inputData->GetRequestedRegion());
	while(!inputIt.IsAtEnd())
	{
		outputIt.Set(inputIt.Get());
		++inputIt;
		++outputIt;
	}
}
void ImageProcessClass::denoise(unsigned int kernerSize)
{
	denoiseLuminescence=NULL;
	denoiseLuminescence=ImageType::New();
	this->kernerSize=kernerSize;
	if (kernerSize==0)
	{
		copyImageData(substractedLuminescence,denoiseLuminescence);
		pseudocolorHighValue=maxValue(denoiseLuminescence);
		pseudocolorLowValue=minValue(denoiseLuminescence);

		luminescenceLabelQImage=NULL;
		int rows=denoiseLuminescence->GetRequestedRegion().GetSize()[0];
		int cols=denoiseLuminescence->GetRequestedRegion().GetSize()[1];
		luminescenceLabelQImage=new QImage(rows,cols,QImage::Format_RGB888);
		ITK2QImage(denoiseLuminescence,luminescenceLabelQImage);//for display

		return;
	}
	MedianFilterType::Pointer medianFilter=MedianFilterType::New();
	ImageType::SizeType filterRadius;
	filterRadius[0]=filterRadius[1]=kernerSize;
	medianFilter->SetInput(substractedLuminescence);
	medianFilter->SetRadius(filterRadius);
	medianFilter->Update();
	denoiseLuminescence=medianFilter->GetOutput();
	pseudocolorHighValue=maxValue(denoiseLuminescence);
	pseudocolorLowValue=minValue(denoiseLuminescence);

	luminescenceLabelQImage=NULL;
	int rows=denoiseLuminescence->GetRequestedRegion().GetSize()[0];
	int cols=denoiseLuminescence->GetRequestedRegion().GetSize()[1];
	luminescenceLabelQImage=new QImage(rows,cols,QImage::Format_RGB888);
	ITK2QImage(denoiseLuminescence,luminescenceLabelQImage);

}
void ImageProcessClass:: pseudocolor(int colormapType,bool hasPhotograph,int highValue,int lowValue)
{
	UnChImageType::PixelType colorMapData[255][3]={
		0,    0,  131,
		0,    0,  135,
		0,    0,  139,
		0,    0,  143,
		0,    0,  147,
		0,    0,  151,
		0,    0,  155,
		0,    0,  159,
		0,    0,  163,
		0,    0,  167,
		0,    0,  171,
		0,    0,  175,
		0,    0,  179,
		0,    0,  183,
		0,    0,  187,
		0,    0,  191,
		0,    0,  195,
		0,    0,  199,
		0,    0,  203,
		0,    0,  207,
		0,    0,  211,
		0,    0,  215,
		0,    0,  219,
		0,    0,  223,
		0,    0,  227,
		0,    0,  231,
		0,    0,  235,
		0,    0,  239,
		0,    0,  243,
		0,    0,  247,
		0,    0,  251,
		0,    0,  255,
		0,    4,  255,
		0,    8,  255,
		0,   12,  255,
		0,   16,  255,
		0,   20,  255,
		0,   24,  255,
		0,   28,  255,
		0,   32,  255,
		0,   36,  255,
		0,   40,  255,
		0,   44,  255,
		0,   48,  255,
		0,   52,  255,
		0,   56,  255,
		0,   60,  255,
		0,   64,  255,
		0,   68,  255,
		0,   72,  255,
		0,   76,  255,
		0,   80,  255,
		0,   84,  255,
		0,   88,  255,
		0,   92,  255,
		0,   96,  255,
		0,  100,  255,
		0,  104,  255,
		0,  108,  255,
		0,  112,  255,
		0,  116,  255,
		0,  120,  255,
		0,  124,  255,
		0,  128,  255,
		0,  131,  255,
		0,  135,  255,
		0,  139,  255,
		0,  143,  255,
		0,  147,  255,
		0,  151,  255,
		0,  155,  255,
		0,  159,  255,
		0,  163,  255,
		0,  167,  255,
		0,  171,  255,
		0,  175,  255,
		0,  179,  255,
		0,  183,  255,
		0,  187,  255,
		0,  191,  255,
		0,  195,  255,
		0,  199,  255,
		0,  203,  255,
		0,  207,  255,
		0,  211,  255,
		0,  215,  255,
		0,  219,  255,
		0,  223,  255,
		0,  227,  255,
		0,  231,  255,
		0,  235,  255,
		0,  239,  255,
		0,  243,  255,
		0,  247,  255,
		0,  251,  255,
		0,  255,  255,
		4,  255,  251,
		8,  255,  247,
		12,  255,  243,
		16,  255,  239,
		20,  255,  235,
		24,  255,  231,
		28,  255,  227,
		32,  255,  223,
		36,  255,  219,
		40,  255,  215,
		44,  255,  211,
		48,  255,  207,
		52,  255,  203,
		56,  255,  199,
		60,  255,  195,
		64,  255,  191,
		68,  255,  187,
		72,  255,  183,
		76,  255,  179,
		80,  255,  175,
		84,  255,  171,
		88,  255,  167,
		92,  255,  163,
		96,  255,  159,
		100,  255,  155,
		104,  255,  151,
		108,  255,  147,
		112,  255,  143,
		116,  255,  139,
		120,  255,  135,
		124,  255,  131,
		128,  255,  128,
		131,  255,  124,
		135,  255,  120,
		139,  255,  116,
		143,  255,  112,
		147,  255,  108,
		151,  255,  104,
		155,  255,  100,
		159,  255,   96,
		163,  255,   92,
		167,  255,   88,
		171,  255,   84,
		175,  255,   80,
		179,  255,   76,
		183,  255,   72,
		187,  255,   68,
		191,  255,   64,
		195,  255,   60,
		199,  255,   56,
		203,  255,   52,
		207,  255,   48,
		211,  255,   44,
		215,  255,   40,
		219,  255,   36,
		223,  255,   32,
		227,  255,   28,
		231,  255,   24,
		235,  255,   20,
		239,  255,   16,
		243,  255,   12,
		247,  255,    8,
		251,  255,    4,
		255,  255,    0,
		255,  251,    0,
		255,  247,    0,
		255,  243,    0,
		255,  239,    0,
		255,  235,    0,
		255,  231,    0,
		255,  227,    0,
		255,  223,    0,
		255,  219,    0,
		255,  215,    0,
		255,  211,    0,
		255,  207,    0,
		255,  203,    0,
		255,  199,    0,
		255,  195,    0,
		255,  191,    0,
		255,  187,    0,
		255,  183,    0,
		255,  179,    0,
		255,  175,    0,
		255,  171,    0,
		255,  167,    0,
		255,  163,    0,
		255,  159,    0,
		255,  155,    0,
		255,  151,    0,
		255,  147,    0,
		255,  143,    0,
		255,  139,    0,
		255,  135,    0,
		255,  131,    0,
		255,  128,    0,
		255,  124,    0,
		255,  120,    0,
		255,  116,    0,
		255,  112,    0,
		255,  108,    0,
		255,  104,    0,
		255,  100,    0,
		255,   96,    0,
		255,   92,    0,
		255,   88,    0,
		255,   84,    0,
		255,   80,    0,
		255,   76,    0,
		255,   72,    0,
		255,   68,    0,
		255,   64,    0,
		255,   60,    0,
		255,   56,    0,
		255,   52,    0,
		255,   48,    0,
		255,   44,    0,
		255,   40,    0,
		255,   36,    0,
		255,   32,    0,
		255,   28,    0,
		255,   24,    0,
		255,   20,    0,
		255,   16,    0,
		255,   12,    0,
		255,    8,    0,
		255,    4,    0,
		255,    0,    0,
		251,    0,    0,
		247,    0,    0,
		243,    0,    0,
		239,    0,    0,
		235,    0,    0,
		231,    0,    0,
		227,    0,    0,
		223,    0,    0,
		219,    0,    0,
		215,    0,    0,
		211,    0,    0,
		207,    0,    0,
		203,    0,    0,
		199,    0,    0,
		195,    0,    0,
		191,    0,    0,
		187,    0,    0,
		183,    0,    0,
		179,    0,    0,
		175,    0,    0,
		171,    0,    0,
		167,    0,    0,
		163,    0,    0,
		159,    0,    0,
		155,    0,    0,
		151,    0,    0,
		147,    0,    0,
		143,    0,    0,
		139,    0,    0,
		135,    0,    0,
		131,    0,    0
	};

	sliced8BitsLuminescence=NULL;
	sliced8BitsLuminescence=UnChImageType::New();
	if(highValue==0&&lowValue==0)
	{
		pseudocolorHighValue=maxValue(denoiseLuminescence);
		pseudocolorLowValue=minValue(denoiseLuminescence);
	}
	else
	{
		pseudocolorHighValue=highValue;
		pseudocolorLowValue=lowValue;
	}
	sliceInputLuminescneceImage(denoiseLuminescence,pseudocolorHighValue,pseudocolorLowValue);//generate a 8 bit luminescence image from denoise image
	pseudocolorLuminescence=NULL;
	pseudocolorLuminescence=RGBImageType::New();
	pseudocolorLuminescenceWithAlpha=NULL;
	pseudocolorLuminescenceWithAlpha=RGBAImageType::New();
	RGBAImageType::IndexType start;
	start[0]=0;
	start[1]=0;
	RGBAImageType::SizeType size;
	size=sliced8BitsLuminescence->GetRequestedRegion().GetSize();
	RGBAImageType::RegionType region;
	region.SetSize(size);
	region.SetIndex(start);
	pseudocolorLuminescenceWithAlpha->SetRegions(region);
	pseudocolorLuminescenceWithAlpha->Allocate();

	RGBFilterType::Pointer rgbFilter=RGBFilterType::New();
	rgbFilter->SetInput(sliced8BitsLuminescence);
	rgbFilter->SetColormap(RGBFilterType::Jet);
	rgbFilter->Update();
	pseudocolorLuminescence=rgbFilter->GetOutput();

	UnChIteratorType it(sliced8BitsLuminescence,sliced8BitsLuminescence->GetRequestedRegion());
	RGBIteratortype RGBIt(pseudocolorLuminescence,pseudocolorLuminescence->GetRequestedRegion());
	RGBPixelType rgbPixelTemp;
	RGBAPixelType rgbaPixelTemp;
	RGBAIteratortype RGBAIt(pseudocolorLuminescenceWithAlpha,pseudocolorLuminescenceWithAlpha->GetRequestedRegion());

	UnChImageType::PixelType uhPixelTemp=0;
	while(!it.IsAtEnd())
	{
		uhPixelTemp=it.Get();
		if(uhPixelTemp>0)
		{
			rgbPixelTemp.SetRed((UnChPixelType)(colorMapData[uhPixelTemp-1][0]));
			rgbPixelTemp.SetGreen((UnChPixelType)(colorMapData[uhPixelTemp-1][1]));
			rgbPixelTemp.SetBlue((UnChPixelType)(colorMapData[uhPixelTemp-1][2] ));
			rgbaPixelTemp.SetRed((UnChPixelType)(colorMapData[uhPixelTemp-1][0] ));
			rgbaPixelTemp.SetGreen((UnChPixelType)(colorMapData[uhPixelTemp-1][1] ));
			rgbaPixelTemp.SetBlue((UnChPixelType)(colorMapData[uhPixelTemp-1][2] ));
			rgbaPixelTemp.SetAlpha(255);
		}
		else
		{
			rgbPixelTemp.SetRed((UnChPixelType)(colorMapData[0][0] ));
			rgbPixelTemp.SetGreen((UnChPixelType)(colorMapData[0][1] ));
			rgbPixelTemp.SetBlue((UnChPixelType)(colorMapData[0][2] ));
			rgbaPixelTemp.SetRed((UnChPixelType)(colorMapData[0][0] ));
			rgbaPixelTemp.SetGreen((UnChPixelType)(colorMapData[0][1] ));
			rgbaPixelTemp.SetBlue((UnChPixelType)(colorMapData[0][2] ));
			rgbaPixelTemp.SetAlpha(0);
		}
		RGBIt.Set(rgbPixelTemp);
		RGBAIt.Set(rgbaPixelTemp);
		++it;
		++RGBIt;
		++RGBAIt;
	}

	if(!hasPhotograph)// if there is not photograph loaded, the fusion image is the pseudocolor image
	{
		fusionImage=NULL;
		fusionImage=RGBImageType::New();
		fusionImage=pseudocolorLuminescence;
		fusionLabelQImage=new QImage(fusionImage->GetRequestedRegion().GetSize()[0],fusionImage->GetRequestedRegion().GetSize()[1],QImage::Format_RGB888);
		ITK2QImage(fusionImage,fusionLabelQImage);
	}
	else//fusion pseudocolor image with photograph
	{
		fusionImage=NULL;
		fusionImage=RGBImageType::New();
		copyImageData(pseudocolorLuminescence,fusionImage);
		RGBIteratortype fusionIt(fusionImage,fusionImage->GetRequestedRegion());
		UnChIteratorType rescaledPhotographIt(rescaledPhotograph,rescaledPhotograph->GetRequestedRegion());
		RGBImageType::PixelType tempPixel;
		it.GoToBegin();
		rescaledPhotographIt.GoToBegin();
		fusionIt.GoToBegin();

		while(!it.IsAtEnd())
		{
			uhPixelTemp=it.Get();
			if(uhPixelTemp==0)
			{
				tempPixel.SetRed(rescaledPhotographIt.Get());
				tempPixel.SetGreen(rescaledPhotographIt.Get());
				tempPixel.SetBlue(rescaledPhotographIt.Get());
				fusionIt.Set(tempPixel);
			}
			++fusionIt;
			++it;
			++rescaledPhotographIt;
				
		}
		fusionLabelQImage=new QImage(fusionImage->GetRequestedRegion().GetSize()[0],fusionImage->GetRequestedRegion().GetSize()[1],QImage::Format_RGB888);
		ITK2QImage(fusionImage,fusionLabelQImage);
	}

	
}
PixelType ImageProcessClass::maxValue(ImageType::Pointer inputImg)
{
	IteratorType it(inputImg,inputImg->GetRequestedRegion());
	it.GoToBegin();
	PixelType temp=0;
	while(!it.IsAtEnd())
	{
		if(temp<it.Get())
			temp=it.Get();
		++it;
	}
	return temp;
}
PixelType ImageProcessClass::minValue(ImageType::Pointer inputImg)
{
	IteratorType it(inputImg,inputImg->GetRequestedRegion());
	it.GoToBegin();
	PixelType temp=65535;
	while(!it.IsAtEnd())
	{
		if(temp>it.Get())
			temp=it.Get();
		++it;
	}
	return temp;
}
void ImageProcessClass::sliceInputLuminescneceImage(ImageType::Pointer inputImage,PixelType HValue,PixelType LValue)//generate a 8 bit luminescence image
{
	sliced8BitsLuminescence=NULL;
	sliced8BitsLuminescence=UnChImageType::New();
	ImageType::Pointer tempImage=ImageType::New();
	copyImageData(inputImage,tempImage);
	PixelType minImageValue,maxImageValue;
	minImageValue=65535;
	maxImageValue=0;
	IteratorType tempIt(tempImage,tempImage->GetRequestedRegion());
	while(!tempIt.IsAtEnd())
	{
		if(tempIt.Get()>=HValue)
			tempIt.Set(HValue);
		if(tempIt.Get()<=LValue)
			tempIt.Set(LValue);
		if(tempIt.Get()>maxImageValue)
			maxImageValue=tempIt.Get();
		if(tempIt.Get()<minImageValue)
			minImageValue=tempIt.Get();
		++tempIt;
	}
	int sliceMin,sliceMax;
	sliceMax=(int)(((float)maxImageValue-LValue)/((float)HValue-LValue)*255);
	sliceMin=(int)(((float)minImageValue-LValue)/((float)HValue-LValue)*255);
	if(sliceMin==0)
		sliceMin=1;//方便进行接下来的矫正，为了避免并不小于lvalue的像素点被划分为0层的情况
	if(sliceMax<2)
		sliceMax=2;
	sliced8BitsLuminescence=rescaleImage(tempImage,sliceMin,sliceMax);
	UnChIteratorType sliceIt(sliced8BitsLuminescence,sliced8BitsLuminescence->GetRequestedRegion());
	ConstIteratorType inputIt(inputImage,inputImage->GetRequestedRegion());
	while(!inputIt.IsAtEnd())
	{
		if(inputIt.Get()<LValue)
		{
			sliceIt.Set(0);
		}
		if(inputIt.Get()>HValue)
		{
			sliceIt.Set(255);
		}
		++inputIt;
		++sliceIt;
	}
}

void ImageProcessClass::saveSingleImage(QString filename,RGBImageType::Pointer inputImage)
{
	string temp1=filename.toStdString();
	const char* temp2=temp1.c_str();
	RGBWriterType::Pointer writer=RGBWriterType::New();
	writer->SetFileName(temp2);
	writer->SetImageIO(tiffIO);
	writer->SetInput(inputImage);
	writer->Update();
}
void ImageProcessClass::saveSingleImage(QString filename,UnChImageType::Pointer inputImage)
{
	string temp1=filename.toStdString();
	const char* temp2=temp1.c_str();
	UnChWriterType::Pointer writer=UnChWriterType::New();
	writer->SetFileName(temp2);
	writer->SetImageIO(tiffIO);
	writer->SetInput(inputImage);
	writer->Update();
}
void ImageProcessClass::saveSingleImage(QString filename,RGBAImageType::Pointer inputImage)
{
	string temp1=filename.toStdString();
	const char* temp2=temp1.c_str();
	RGBAWriterType::Pointer writer=RGBAWriterType::New();
	writer->SetFileName(temp2);
	writer->SetImageIO(tiffIO);
	writer->SetInput(inputImage);
	writer->Update();
}
void ImageProcessClass::saveSingleImage(QString filename,ImageType::Pointer inputImage)
{
	string temp1=filename.toStdString();
	const char* temp2=temp1.c_str();
	WriterType::Pointer writer=WriterType::New();
	writer->SetFileName(temp2);
	writer->SetImageIO(tiffIO);
	writer->SetInput(inputImage);
	writer->Update();
}
void ImageProcessClass::writeImage(bool saveAllImages,QString filename)
{
	if (saveAllImages)
	{
		QString dirname;
		QStringList list=filename.split(".");
		dirname=list[0];
		QDir tempDir=QDir();
		bool asdf=tempDir.exists(dirname);
		if(!tempDir.exists(dirname))
		{
			if(!tempDir.mkpath(dirname))
			{
				QMessageBox::information(NULL,"Warning","Save images failed! Dir cannot be created!");
				return;
			}
			saveSingleImage(QString(dirname+QString("/originalLuminescence.tif")),originalLuminesence);
			saveSingleImage(QString(dirname+QString("/fusionImage.tif")),fusionImage);
			saveSingleImage(QString(dirname+QString("/sliced8BitsLuminescence.tif")),sliced8BitsLuminescence);
			saveSingleImage(QString(dirname+QString("/psedudocolorImage.tif")),pseudocolorLuminescence);
			saveSingleImage(QString(dirname+QString("/pseudocolorImageWithAlpha.tif")),pseudocolorLuminescenceWithAlpha);
			saveSingleImage(QString(dirname+QString("/substractedBackgroundLuminescence.tif")),substractedLuminescence);
			saveSingleImage(QString(dirname+QString("/denoiseLuminescence.tif")),denoiseLuminescence);
			saveSingleImage(QString(dirname+QString("/resizedLuminesence.tif")),resizedLuminesence);

			saveSingleImage(QString(dirname+QString("/resizedPhotograph.tif")),resizedPhotograph);
			saveSingleImage(QString(dirname+QString("/originalPhotograph.tif")),originalPhotograph);
			saveSingleImage(QString(dirname+QString("/rescaledPhotoraph.tif")),rescaledPhotograph);
			
		}
		warningMessage=QString("All images saved!\n")+QString("Folder path: ")+QString(dirname)+QString("\n");
	} 
	else
	{
		saveSingleImage(filename,fusionImage);
		warningMessage=QString("Fusion image saved!\n")+QString("FileName: ")+QString(filename)+QString("\n");
	}
}
void ImageProcessClass::setResizeStrategy(bool resizeFlag)
{
	this->resizeFlag=resizeFlag;
}

ImageType::Pointer ImageProcessClass::Mat2ITK(Mat inputImg,ImageType::Pointer outputImg)
{
	ImageType::IndexType start;
	ImageType::SizeType size;
	start[0]=start[1]=0;
	size[0]=inputImg.rows;
	size[1]=inputImg.cols;
	ImageType::RegionType region;
	region.SetSize(size);
	region.SetIndex(start);
	outputImg->SetRegions(region);
	outputImg->Allocate();
	/********************************************************/
		//#define CV_8U   0
		//#define CV_8S   1
		//#define CV_16U  2
		//#define CV_16S  3
		//#define CV_32S  4
		//#define CV_32F  5
		//#define CV_64F  6
	//目前只有两种，即0：8位，2：16位
	/********************************************************/
	if (inputImg.type()==0)
	{
		MatConstIterator_<unsigned char> it=inputImg.begin<unsigned char>(),it_end=inputImg.end<unsigned char>();
		IteratorType outIt(outputImg,outputImg->GetRequestedRegion());
		outIt.GoToBegin();
		while(it!=it_end)
		{
			outIt.Set(*it);
			++it;
			++outIt;
		}
	} 
	else
	{
		MatConstIterator_<unsigned short> it=inputImg.begin<unsigned short>(),it_end=inputImg.end<unsigned short>();
		IteratorType outIt(outputImg,outputImg->GetRequestedRegion());
		outIt.GoToBegin();
		while(it!=it_end)
		{
			outIt.Set(*it);
			++it;
			++outIt;
		}
	}
	
	return outputImg;
}


bool ImageProcessClass::isLuminescenceLargerThanPhotograph()
{
	if (originalLuminesenceMat.rows>=originalPhotographMat.rows)
	{
		return true;
	} 
	else
	{
		return false;
	}
}
bool ImageProcessClass::isDenoiseLargeEqualToResizedPhotograph()
{
	if (denoiseLuminescence->GetRequestedRegion().GetSize()[0]==resizedPhotograph->GetRequestedRegion().GetSize()[0])
	{
		return true;
	} 
	else
	{
		return false;
	}
}