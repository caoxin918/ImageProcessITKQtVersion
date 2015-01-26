#include "imageprocessitkqtversion.h"
#include "QFileDialog"
#include "QMessageBox"
#include "qpixmap.h"

ImageProcessITKQtVersion::ImageProcessITKQtVersion(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	imgProcess=ImageProcessClass();
	clearAll();
	warningMessage="";
	QObject::connect(ui.pushButton_luminescence,SIGNAL(clicked()),this,SLOT(luminescence_clicked()));
	QObject::connect(ui.pushButton_photograph,SIGNAL(clicked()),this,SLOT(photograph_clicked()));
	QObject::connect(ui.pushButton_clear,SIGNAL(clicked()),this,SLOT(clear()));
	QObject::connect(ui.pushButton_denoise,SIGNAL(clicked()),this,SLOT(denoise_clicked()));
	QObject::connect(ui.pushButton_substract,SIGNAL(clicked()),this,SLOT(subtract_clicked()));
	QObject::connect(ui.verticalSlider_highValue,SIGNAL(valueChanged(int)),this,SLOT(sliderOrspinboxOfPseudocolorLowValueChanged(int)));
	QObject::connect(ui.verticalSlider_lowValue,SIGNAL(valueChanged(int)),this,SLOT(sliderOrspinboxOfPseudocolorLowValueChanged(int)));
	QObject::connect(ui.spinBox_highValue,SIGNAL(valueChanged(int)),this,SLOT(sliderOrspinboxOfPseudocolorLowValueChanged(int)));
	QObject::connect(ui.spinBox_lowValue,SIGNAL(valueChanged(int)),this,SLOT(sliderOrspinboxOfPseudocolorLowValueChanged(int)));
	QObject::connect(ui.pushButton_save,SIGNAL(clicked()),this,SLOT(saveFiles()));
}

ImageProcessITKQtVersion::~ImageProcessITKQtVersion()
{

}
void ImageProcessITKQtVersion::clear()
{
	clearAll();
}
void ImageProcessITKQtVersion::clearAll()
{
	imgProcess.initial();

	luminescenceFlag=false;
	photoraphFlag=false;
	substractFlag=false;
	denoiseFlag=false;

	ui.label_luminescence->clear();
	ui.label_fusion->clear();
	ui.label_white->clear();
	ui.spinBox_background->setValue(0);
	ui.spinBox_filterSize->setValue(0);
	ui.spinBox_lowValue->setValue(0);
	ui.spinBox_highValue->setValue(65535);
	ui.textEdit_messageBox->setText("");
}

void ImageProcessITKQtVersion::luminescence_clicked()
{
	clearAll();
	QString path=QFileDialog::getOpenFileName(this,"Open luminescence",".","tiff Files(*.tif)");
// 	if(path.isEmpty())
// 	{
// 		return;
// 	}
	if(!imgProcess.readLuminescence(path))
	{
		ui.textEdit_messageBox->append(imgProcess.getWarningMessage());
		return;
	}
	showInLuminescenceLabel(imgProcess.getLuminescenceLabelQImage());
	ui.textEdit_messageBox->append(imgProcess.getWarningMessage());
	luminescenceFlag=true;
}
void ImageProcessITKQtVersion::photograph_clicked()
{
	photoraphFlag=false;
	ui.label_white->clear();

	QString path=QFileDialog::getOpenFileName(this,"Open photograph",".","tiff Files(*.tif)");
	if(ui.checkBox_resize->isChecked())
	{
		imgProcess.setResizeStrategy(true);
		
	}
	else
	{
		imgProcess.setResizeStrategy(false);
	}
	if(!imgProcess.readPhotograph(path))
	{
		ui.textEdit_messageBox->append(imgProcess.getWarningMessage());
		return;
	}

	showInPhotographLabel(imgProcess.getPhotographLabelQImage());
	ui.textEdit_messageBox->append(imgProcess.getWarningMessage());

	photoraphFlag=true;

	if(denoiseFlag)//这里，判断导入后的白光图像和上一回伪彩色后的图像大小是否一致，弱不一致，则重新进行减去背景值-滤波-伪彩色这一流程，反之则直接进行伪彩色叠加
	{
		if (imgProcess.isDenoiseLargeEqualToResizedPhotograph())
		{
			psedudoColor(0,photoraphFlag,ui.verticalSlider_highValue->value(),ui.verticalSlider_lowValue->value());
			showInFusionLabel(imgProcess.getFusionLabelQImage());
		} 
		else
		{
			ui.label_fusion->clear();
			imgProcess.substract(ui.spinBox_background->value());
			imgProcess.denoise(ui.spinBox_filterSize->value());
			ui.verticalSlider_highValue->setValue(imgProcess.getHighValue());
			ui.verticalSlider_lowValue->setValue(imgProcess.getLowValue());
			showInFusionLabel(imgProcess.getFusionLabelQImage());
			substractFlag=true;
			denoiseFlag=true;
		}
	}
}


void ImageProcessITKQtVersion::showInLuminescenceLabel(QImage* img)
{
	QImage* imgScaled=new QImage();
	*imgScaled=img->scaled(ui.label_luminescence->width(),ui.label_luminescence->height(),Qt::KeepAspectRatio);
	ui.label_luminescence->setPixmap(QPixmap::fromImage(*imgScaled));
}
void ImageProcessITKQtVersion::showInPhotographLabel(QImage* img)
{
	QImage* imgScaled=new QImage();
	*imgScaled=img->scaled(ui.label_white->width(),ui.label_white->height(),Qt::KeepAspectRatio);
	ui.label_white->setPixmap(QPixmap::fromImage(*imgScaled));
}
void ImageProcessITKQtVersion::showInFusionLabel(QImage* img)
{
	QImage* imgScaled=new QImage();
	*imgScaled=img->scaled(ui.label_fusion->width(),ui.label_fusion->height(),Qt::KeepAspectRatio);
	ui.label_fusion->setPixmap(QPixmap::fromImage(*imgScaled));
}
void ImageProcessITKQtVersion::subtract_clicked()
{
	substractFlag=false;
	if (!luminescenceFlag)
	{
		warningMessage=QString("Please load luminescence before this step!")+QString("\n");
		ui.textEdit_messageBox->append(warningMessage);
		return;
	}
	
	imgProcess.substract(ui.spinBox_background->value());
	showInLuminescenceLabel(imgProcess.getLuminescenceLabelQImage());

	warningMessage=QString("Background Removed!\n")+QString("BackgroundValue: ")+QString::number(ui.spinBox_background->value())+QString("\n");
	ui.textEdit_messageBox->append(warningMessage);
	substractFlag=true;
}
void ImageProcessITKQtVersion::denoise_clicked()
{
	denoiseFlag=false;
	if (!substractFlag)
	{
		warningMessage=QString("Please substract background before this step!")+QString("\n");
		ui.textEdit_messageBox->append(warningMessage);
		return;
	}
	imgProcess.denoise(ui.spinBox_filterSize->value());
	showInLuminescenceLabel(imgProcess.getLuminescenceLabelQImage());

//	psedudoColor(0,photoraphFlag,ui.spinBox_highValue->value(),ui.spinBox_lowValue->value());
	denoiseFlag=true;
	ui.verticalSlider_highValue->setValue(imgProcess.getHighValue());
	ui.verticalSlider_lowValue->setValue(imgProcess.getLowValue());
	warningMessage=QString("Denoise done!")+QString("\n");
	if(ui.spinBox_filterSize->value()!=0)
	{
		warningMessage+=QString("Denoise Kernel: ")+QString::number(ui.spinBox_filterSize->value())+QString("\n");
	}
	ui.textEdit_messageBox->append(warningMessage);
	
	
}
void ImageProcessITKQtVersion::sliderOrspinboxOfPseudocolorHighValueChanged(int highValue)
{
	if(!denoiseFlag)
		return;
// 	ui.verticalSlider_lowValue->setRange(0,highValue-1);
// 	ui.spinBox_lowValue->setRange(0,highValue-1);
// 	if (highValue<=ui.verticalSlider_lowValue->value())
// 	{
// 		ui.verticalSlider_highValue->setValue(ui.verticalSlider_lowValue->value()+10);
// 		ui.verticalSlider_highValue->update();
//  	}
	psedudoColor(0,photoraphFlag,ui.verticalSlider_highValue->value(),ui.verticalSlider_lowValue->value());
	showInFusionLabel(imgProcess.getFusionLabelQImage());
}
void ImageProcessITKQtVersion::sliderOrspinboxOfPseudocolorLowValueChanged(int lowValue)
{
	if(!denoiseFlag)
		return;
	psedudoColor(0,photoraphFlag,ui.verticalSlider_highValue->value(),ui.verticalSlider_lowValue->value());
	showInFusionLabel(imgProcess.getFusionLabelQImage());
}
void ImageProcessITKQtVersion::psedudoColor(int colormapType,bool hasPhotoraph,int highValue,int lowValue)
{
	imgProcess.pseudocolor(colormapType,hasPhotoraph,highValue,lowValue);
}
void ImageProcessITKQtVersion::saveFiles()
{
	if(!denoiseFlag)
	{
		ui.textEdit_messageBox->append("Please denoise before Save file(s)!\n");
		return;
	}
	if(!ui.checkBox_saveAll->isChecked())//just store the fusion image
	{
		QString filename=QFileDialog::getSaveFileName(this,tr("Save image"),".","tiff Files(*.tif)");
		if(filename.isEmpty())
		{
			ui.textEdit_messageBox->append("Canceled!\n");
			return;
		}
		
		imgProcess.writeImage(false,filename);
		ui.textEdit_messageBox->append(imgProcess.getWarningMessage());
	}
	else//store all the images in a file dir
	{
		QString filename=QFileDialog::getSaveFileName(this,tr("Create a folder to save images"),".","tiff Files(*.suffix)");
		if(filename.isEmpty())
		{
			ui.textEdit_messageBox->append("Canceled!\n");
			return;
		}
		
		imgProcess.writeImage(true,filename);
		ui.textEdit_messageBox->append(imgProcess.getWarningMessage());
	}
}