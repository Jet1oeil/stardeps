/*
 * CommandsExecutor.cpp
 *
 *  Created on: 11 juin 2020
 *      Author: ebeuque
 */

#include "CommandEnvironment/AbstractCommandEnvironment.h"
#include "CommandEnvironment/DefaultCommandEnvironment.h"
#include "CommandEnvironment/PackageCommandEnvironment.h"

#include "Commands/CommandCreateEnv.h"
#include "Commands/CommandPrepare.h"
#include "Commands/CommandConfigure.h"
#include "Commands/CommandBuild.h"
#include "Commands/CommandInstall.h"
#include "Commands/CommandClean.h"

#include "CommandsExecutor.h"

CommandsExecutor::CommandsExecutor()
{
	m_pCommandEnv = NULL;
}

CommandsExecutor::~CommandsExecutor()
{
	doClearCommands();

	if(m_pCommandEnv){
		delete m_pCommandEnv;
		m_pCommandEnv = NULL;
	}
}

void CommandsExecutor::doClearCommands()
{
	AbstractCommand* pCommand;
	while(!m_listCommands.isEmpty()){
		pCommand = m_listCommands.takeFirst();
		delete pCommand;
	}
}

bool CommandsExecutor::prepareCommands(const QString& szCommand, int argc, char** argv)
{
	bool bRes = false;

	bool bPackageCommand = false;

	bool bPreviousStep = false;

	QString szArg;
	for(int i=0; i<argc; i++)
	{
		szArg = argv[i];
		if(szArg == "--previous-steps"){
			bPreviousStep = true;
		}
	}

	// Fill the list of commands
	if(szCommand == "createenv"){
		m_listCommands.append(new CommandCreateEnv());
	}else if(szCommand == "prepare"){
		m_listCommands.append(new CommandPrepare());
		bPackageCommand = true;
	}else if(szCommand == "configure"){
		if(bPreviousStep){
			m_listCommands.append(new CommandPrepare());
		}
		m_listCommands.append(new CommandConfigure());
		bPackageCommand = true;
	}else if(szCommand == "build"){
		if(bPreviousStep){
			m_listCommands.append(new CommandPrepare());
			m_listCommands.append(new CommandConfigure());
		}
		m_listCommands.append(new CommandBuild());
		bPackageCommand = true;
	}else if(szCommand == "install"){
		if(bPreviousStep){
			m_listCommands.append(new CommandPrepare());
			m_listCommands.append(new CommandConfigure());
			m_listCommands.append(new CommandBuild());
		}
		m_listCommands.append(new CommandInstall());
		bPackageCommand = true;
	}else if(szCommand == "clean"){
		m_listCommands.append(new CommandClean());
		bPackageCommand = true;
	}

	// Prepare command environment
	if(!m_listCommands.isEmpty()){
		bRes = true;

		// Initialize commands env
		if(bPackageCommand){
			m_pCommandEnv = new PackageCommandEnvironment();
		}else{
			m_pCommandEnv = new DefaultCommandEnvironment();
		}
		m_pCommandEnv->setVirtualEnvironmentPath(QDir("."));

		bRes = m_pCommandEnv->init(argc, argv);
	}

	// Prepare the commands
	if(bRes){
		QList<AbstractCommand *>::iterator iter;
		for(iter = m_listCommands.begin(); iter != m_listCommands.end(); ++iter)
		{
			AbstractCommand* pCommand = (*iter);
			pCommand->setCommandEnvironment(m_pCommandEnv);

			bRes = pCommand->init(argc, argv);
			if(!bRes){
				break;
			}
		}
	}

	// Clear commands on fail
	if(!bRes){
		doClearCommands();
	}

	return bRes;
}

bool CommandsExecutor::executeCommands()
{
	bool bRes = true;

	QList<AbstractCommand*>::iterator iter;

	// Run
	if(m_pCommandEnv){
		bRes = m_pCommandEnv->load();
	}

	// Execute commands list
	if(bRes){
		for(iter = m_listCommands.begin(); iter != m_listCommands.end(); ++iter)
		{
			AbstractCommand* pCommand = (*iter);
			bRes = pCommand->execute();
			if(!bRes){
				break;
			}
		}
	}

	return bRes;
}
