#include <QCoreApplication>
#include <QFile>

#include <QDebug>
#include <QFileInfo>

#include "decompiler/codedisassembler.h"
#include "rom/romutil.h"

#include "codetemplatelibrary.h"
#include "valuelibrary.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "assembler/codeassembler.h"

int main(int argc, char** argv) {
	QCoreApplication app(argc, argv);
	Q_UNUSED(app);

	bool decompMode = true;
	QString romFile = "FE5_J.sfc";
	quint64 offset = 0x3E8000;
	QString decompType = "eventDefinition";
	QString fileName = "out.tea";

	{
		QStringList list = QCoreApplication::arguments();

		for (int i=0; i<list.size()-1; ++i) {
			if (list[i] == "disassemble")
				decompMode = true;
			else if (list[i] == "assemble")
				decompMode = false;
			else if (list[i] == "-rom")
				romFile = list[++i];
			else if (list[i] == "-offset")
				offset = list[++i].toLongLong(nullptr, 0);
			else if (list[i] == "-type")
				decompType = list[++i];
			else if (list[i] == "-file")
				fileName = list[++i];
		}
	}

	QFile file(fileName);

	tea::ROM rom;
	rom.loadFromFile(romFile);

	tea::CodeTemplateLibrary lib;
	lib.loadFromDir("./code/");

	tea::ValueLibrary valLib;
	valLib.loadFromDir("./code/");

	if (decompMode) {
		tea::CodeDisassembler decompiler(&lib, &valLib);
		decompiler.disassemble(rom.midRef(offset), decompType);

		if (!file.open(QIODevice::WriteOnly))
			return 1;

		QTextStream stream(&file);
		decompiler.printOutput(&stream);

		file.close();
	} else {
		if (!file.open(QIODevice::ReadOnly))
			return 1;

		QString data = file.readAll();
		file.close();

		tea::Lexer lexer;
		tea::Parser parser(&lib, &lexer);
		tea::CodeAssembler assembler(&rom, &valLib);

		QObject::connect(&lexer, &tea::Lexer::tokenReady, &parser, &tea::Parser::handleToken);
		QObject::connect(&parser, &tea::Parser::expressionReady, &assembler, &tea::CodeAssembler::handleExpression);

		QObject::connect(&lexer, &tea::Lexer::tokenError, [] (QStringRef, QString what) {
			qDebug() << what;
		});

		QObject::connect(&parser, &tea::Parser::parseError, [] (QString what) {
			qDebug() << what;
		});

		lexer.tokenize(data.midRef(0));
		assembler.outputToFile(QFileInfo(romFile).path() + "tea_out.sfc");
	}

	return 0;
}