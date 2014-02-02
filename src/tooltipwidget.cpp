#include "tooltipwidget.h"
#include <QStringList>
#include <QLabel>

TooltipWidget::TooltipWidget (QWidget *parent) : QStackedWidget (parent)
{
	setAutoFillBackground (true);
}

void TooltipWidget::setTooltips (const QStringList & tooltipsContent)
{
	QWidget *tmpWidget = 0;
	int index = 0;

	// Delete all tooltips currently in the stacked widget
	while ((tmpWidget = currentWidget ())) {
		removeWidget (tmpWidget);
		delete tmpWidget;
	}

	// Create the new tooltips
	for (const QString &content:tooltipsContent) {
		QLabel *labelTooltipContent;

		// Insert the navigation text (page number and previous/next page) if needed
		if (tooltipsContent.count () > 1) {
			labelTooltipContent = new QLabel (content +
					QString ("<br/><br/>%1/%2 <br/>F2:← F3:→").arg (index + 1).arg (tooltipsContent.count ()));
		} else {
			labelTooltipContent = new QLabel (content);
		}
		labelTooltipContent->setStyleSheet ("QLabel { border: 1px solid black; border-radius: 5px; }");
		labelTooltipContent->setAlignment (Qt::AlignTop);
		addWidget (labelTooltipContent);
		++index;
	}
}

void TooltipWidget::nextTooltip ()
{
	if (count ()) {
		setCurrentIndex ((currentIndex () + 1) % count ());
	}
}

void TooltipWidget::previousTooltip ()
{
	if (count ()) {
		setCurrentIndex (currentIndex () == 0 ? count () - 1 : currentIndex () - 1);
	}
}


