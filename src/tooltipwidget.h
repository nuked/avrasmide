#ifndef TOOLTIPWIDGET_H
#define TOOLTIPWIDGET_H

#include <QStackedWidget>

class QStringList;

class TooltipWidget : public QStackedWidget
{
	Q_OBJECT
public:
	explicit TooltipWidget (QWidget *parent = 0);

	void setTooltips (const QStringList &tooltipsContent);
	void nextTooltip ();
	void previousTooltip ();
};

#endif // TOOLTIPWIDGET_H

