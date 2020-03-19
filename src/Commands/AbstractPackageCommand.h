/*
 * AbstractPackageCommand.h
 *
 *  Created on: 18 mars 2020
 *      Author: ebeuque
 */

#ifndef SRC_COMMANDS_ABSTRACTPACKAGECOMMAND_H_
#define SRC_COMMANDS_ABSTRACTPACKAGECOMMAND_H_

#include "AbstractCommand.h"

class AbstractPackageCommand : public AbstractCommand
{
public:
	AbstractPackageCommand(const QString& szLabel);
	virtual ~AbstractPackageCommand();

	void setPackageName(const QString& szPackageName);
	void setVersion(const QString& szVersion);

protected:
	QString m_szPackageName;
	QString m_szVersion;
};

#endif /* SRC_COMMANDS_ABSTRACTPACKAGECOMMAND_H_ */