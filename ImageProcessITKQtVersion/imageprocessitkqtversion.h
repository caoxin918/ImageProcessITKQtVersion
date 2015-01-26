#ifndef IMAGEPROCESSITKQTVERSION_H
#define IMAGEPROCESSITKQTVERSION_H

#include <QtGui/QMainWindow>
#include <QDir>
#include "ui_imageprocessitkqtversion.h"
#include "imageProcessClass.h"

class ImageProcessITKQtVersion : public QMainWindow
{
	Q_OBJECT

public:
	ImageProcessITKQtVersion(QWidget *parent = 0, Qt::WFlags flags = 0);
	~ImageProcessITKQtVersion();
	
private slots:
	void luminescence_clicked();
	void photograph_clicked();
	void clear();
	void subtract_clicked();
	void denoise_clicked();
	void sliderOrspinboxOfPseudocolorLowValueChanged(int);
	void sliderOrspinboxOfPseudocolorHighValueChanged(int);
	void saveFiles();
// 	void on_pushButton_substract_clicked();
// 	void on_pushButton_denoise_clicked();
private:
	Ui::ImageProcessITKQtVersionClass ui;
	ImageProcessClass imgProcess;
	void showInLuminescenceLabel(QImage* img);
	void showInPhotographLabel(QImage* img);
	void showInFusionLabel(QImage* img);
	void clearAll();
	void psedudoColor(int colormapType,bool hasPhotoraph,int highValue,int lowValue);
private:
	bool luminescenceFlag;
	bool photoraphFlag;
	bool substractFlag;
	bool denoiseFlag;

	QString warningMessage;
};

#endif // IMAGEPROCESSITKQTVERSION_H
