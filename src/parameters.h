#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QDialog>
#include <QList>
#include <QListView>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>

#include "editorconfiguration.h"
#include "arduinoconfiguration.h"
#include "optionconfig.h"

namespace Ui
{
	class Parameters;
}

class Parameters:public QDialog
{
	Q_OBJECT private slots:void chgOption (QListWidgetItem *);
	void okClicked ();
	void cancelClicked ();
	void applyClicked ();
public:
	static Parameters &getInstance (bool reset = false) {
		static Parameters *instance = 0;

		if (reset) {
			delete instance;
			instance = new Parameters ();
		}
		return *instance;
	}

	ArduinoConfiguration *arduinoConfig ();
	EditorConfiguration *editorConfig ();

private:
	explicit Parameters (QWidget * parent = 0);
	~Parameters ();

	void buildLayout ();
	void initializeOptions ();
	void readSettings ();
	void writeSettings ();
	OptionConfig *findByName (QString);
	Ui::Parameters * ui;
	EditorConfiguration *_editorConfig;
	ArduinoConfiguration *_arduinoConfig;
	QListWidget *_optionListWidget;
	QList<OptionConfig *>*_optionList;
	QStackedWidget *_widgetStack;
	QPushButton *_okButton;
	QPushButton *_cancelButton;
	QPushButton *_applyButton;
};


#endif // PARAMETERS_H

