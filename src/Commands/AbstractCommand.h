/*
 * AbstractCommand.h
 *
 *  Created on: 13 mars 2020
 *      Author: ebeuque
 */

#ifndef SRC_COMMANDS_ABSTRACTCOMMAND_H_
#define SRC_COMMANDS_ABSTRACTCOMMAND_H_

#include <QString>
#include <QDir>
#include <QSharedPointer>

#include "Environment/Environment.h"
#include "Shell/ShellExecutor.h"
#include "Formulas/Formula.h"

class AbstractCommandEnvironment;

class AbstractCommand
{
public:
	AbstractCommand(const QString& szLabel);
	virtual ~AbstractCommand();

	void setCommandEnvironment(AbstractCommandEnvironment* pCmdEnd);
	AbstractCommandEnvironment* getCommandEnvironment() const;

    Environment& getEnv();
    const Environment& getEnv() const;

	const QDir& getVirtualEnvironmentPath() const;

	const QDir& getWorkingDirectory(const QDir& dirDefault) const;

	bool init(int argc, char**argv);

	bool execute();

protected:
	virtual bool doProcessArgument(int i, const QString& szArg);
	virtual bool doExecute() = 0;

protected:
	QString m_szLabel;

	AbstractCommandEnvironment* m_pCommandEnvironment;

	QDir m_dirCurrentWorkingDirectory;
	ShellExecutor m_shell;
};

#endif /* SRC_COMMANDS_ABSTRACTCOMMAND_H_ */
