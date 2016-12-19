//##########################################################################
//#                                                                        #
//#                            CLOUDCOMPARE                                #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#ifndef CC_SF_EDIT_DIALOG_HEADER
#define CC_SF_EDIT_DIALOG_HEADER

#include <QWidget>

#include <ui_sfEditDlg.h>

class ccScalarField;

//! GUI scalar field interactor for properties list dialog
class sfEditDlg : public QWidget, public Ui::SFEditDlg
{
    Q_OBJECT

public:

	//! Default constructor
    sfEditDlg(QWidget* parent=0);

	//! Updates dialog with a given scalar field
    void fillDialogWith(ccScalarField* sf);

public slots:

    void minValSBChanged(double val);
    void maxValSBChanged(double val);
    void minSatSBChanged(double val);
    void maxSatSBChanged(double val);

	void dispValSliderChanged(int,int);
	void satValSliderChanged(int,int);

	void nanInGrayChanged(bool);
	void alwaysShow0Changed(bool);
	void symmetricalScaleChanged(bool);
	void logScaleChanged(bool);

signals:

	//! Signal emitted when the SF display parameters have changed
    void entitySFHasChanged();

protected:

	//conversionb between sliders (integer) and checkbox (double) values
    double dispSpin2slider(double val) const;
    double satSpin2slider(double val) const;
    double dispSlider2spin(int pos) const;
    double satSlider2spin(int pos) const;

	//! Associated scalar field
	ccScalarField* m_associatedSF;
};

#endif //CC_SF_EDIT_DIALOG_HEADER
