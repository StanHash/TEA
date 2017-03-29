#ifndef TEA_PREPROCESSOR_H
#define TEA_PREPROCESSOR_H

#include <QObject>

#include "lang/error/assemblyexception.h"
#include "lang/core/assemblyline.h"

namespace tea {

class Preprocessor : public QObject {
	Q_OBJECT
public:
	Preprocessor();

	void processLine(uint lineNumber, QString line);

signals:
	void lineReady(tea::AssemblyLine line);
	void finished();
	void error(tea::AssemblyException exception);

public slots:
	void handleFile(QString fileName);

private:
	int mCurrentIncludeDepth;
	QString mCurrentFile;
};

} // namespace tea

#endif // TEA_PREPROCESSOR_H