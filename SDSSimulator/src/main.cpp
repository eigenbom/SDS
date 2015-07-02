#include <QApplication>
#include "simulator.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	Simulator sim(&app,argc,argv);

	if (sim.run())
	{
		sim.show();
		return app.exec();
	}
	else
		return -1;
}








