/*
 * CommandPrepare.cpp
 *
 *  Created on: 13 mars 2020
 *      Author: ebeuque
 */

#include "Connector/ConnectorArchive.h"
#include "Connector/ConnectorGit.h"
#include "Connector/ConnectorSVN.h"
#include "Connector/ConnectorMercurial.h"

#include "CommandPrepare.h"

CommandPrepare::CommandPrepare() : AbstractPackageCommand("prepare")
{

}

CommandPrepare::~CommandPrepare()
{

}

void CommandPrepare::setScmBranchVersion(const QString& szVersion)
{
	m_szScmBranchVersion = szVersion;
}

void CommandPrepare::setScmTagVersion(const QString& szVersion)
{
	m_szScmBranchVersion = szVersion;
}

bool CommandPrepare::doProcessArgument(int i, const QString& szArg)
{
	if(szArg.startsWith("--scm-branch-version=")){
		setScmBranchVersion(szArg.mid(21));
		return true;
	}
	if(szArg.startsWith("--scm-tag-version=")){
		setScmTagVersion(szArg.mid(18));
		return true;
	}
	return AbstractPackageCommand::doProcessArgument(i, szArg);
}

bool CommandPrepare::doExecute()
{
	bool bRes = true;

	QDir dirSrcPackage = getSourcePackageDir();

	const QSharedPointer<Formula>& pFormula = getFormula();

	// Get sources
	if(bRes){
		bRes = prepareSources(pFormula, dirSrcPackage);
	}

	// Configure the version
	if(bRes && !getConfigureVersion().isEmpty()){
		bRes = configureVersion(pFormula, dirSrcPackage);
	}

	// Execute steps
	if(bRes){
	    bRes = doExecuteStep("prepare", getSourcePackageDir());
	}

    // Execute commands
    if(bRes){
        const FormulaRecipeList& listRecipes = getFormula()->getRecipeList();
        QString szTargetPlateform = getEnv().getPlatformTypeName();
        if(listRecipes.contains(szTargetPlateform)){
            const FormulaRecipe& recipe = listRecipes.value(szTargetPlateform);
            const FormulaCommands& listCommands = recipe.getPrepareCommands();

            FormulaCommands::const_iterator iter_cmd;
            for(iter_cmd = listCommands.constBegin(); iter_cmd != listCommands.constEnd(); ++iter_cmd)
            {
                bRes = doRunCommand((*iter_cmd), getSourcePackageDir());
                if(!bRes){
                    qCritical("[prepare] aborting prepare due to error");
                    break;
                }
            }
        }
    }

	return bRes;
}

bool CommandPrepare::prepareSources(const QSharedPointer<Formula>& pFormula, const QDir& dirWorkingCopy)
{
	bool bRes = true;

	if(pFormula->getTypeSCM() == Formula::SCM_Git)
	{
		ConnectorGit connector(getEnv());
		if(dirWorkingCopy.exists()){
		    QString szBranch = "master";
		    if(!pFormula->getSCMDefaultBranch().isEmpty()){
                szBranch = pFormula->getSCMDefaultBranch();
		    }
			bRes = connector.git_checkout(szBranch, dirWorkingCopy);
			bRes = connector.git_pull(dirWorkingCopy);
		}else{
			bRes = connector.git_clone(pFormula->getSCMURL(), dirWorkingCopy);
		}
    }else if(pFormula->getTypeSCM() == Formula::SCM_Svn)
    {
        ConnectorSVN connector(getEnv());
        if(!dirWorkingCopy.exists()) {
            QString szBranch = "trunk";
            if (!pFormula->getSCMDefaultBranch().isEmpty()) {
                szBranch = pFormula->getSCMDefaultBranch();
            }
            if (!m_szScmBranchVersion.isEmpty()) {
                szBranch = "branch/" + m_szScmBranchVersion;
            }
            if (!m_szScmTagVersion.isEmpty()) {
                szBranch = "tags/" + m_szScmTagVersion;
            }
            bRes = connector.svn_checkout(pFormula->getSCMURL() + "/" + szBranch, dirWorkingCopy);
        }else{
            bRes = connector.svn_update(dirWorkingCopy);
        }
	}else if(pFormula->getTypeSCM() == Formula::SCM_Mercurial)
	{
		ConnectorMercurial connector(getEnv());
		if(dirWorkingCopy.exists()){
			QString szBranch = "default";
			if(!pFormula->getSCMDefaultBranch().isEmpty()){
				szBranch = pFormula->getSCMDefaultBranch();
			}
			bRes = connector.hg_checkout(szBranch, dirWorkingCopy);
			bRes = connector.hg_update(dirWorkingCopy);
		}else{
			bRes = connector.hg_clone(pFormula->getSCMURL(), dirWorkingCopy);
		}
    }else if(pFormula->getTypeSCM() == Formula::SCM_Archive)
    {
        ConnectorArchive conector(getEnv());

		QString szSCMUrl = pFormula->getSCMURL();

		szSCMUrl = szSCMUrl.replace("${VERSION}", getConfigureVersion());

        // Create archive directory
        QDir dirArchive = getSourceArchivePackageDir();
        if(!dirArchive.exists()){
            bRes = dirWorkingCopy.mkpath(".");
        }

        // Download archive
        QString szArchiveFile;
        if(bRes){
            QString szFileName;
            if(pFormula->getSCMURL().endsWith("tar.gz")){
                szFileName = getPackageNameVersion() + ".tar.gz";
            }
            bRes = conector.archive_download(szSCMUrl, dirArchive, szArchiveFile, szFileName);
        }

        // Extract archive directory
        if(bRes){
            if(!dirWorkingCopy.exists()){
                bRes = dirWorkingCopy.mkpath(".");
            }
        }
        if(bRes){
            bRes = conector.archive_extract(szArchiveFile, dirWorkingCopy);
        }
    }

	return bRes;
}

bool CommandPrepare::configureVersion(const QSharedPointer<Formula>& pFormula, const QDir& dirWorkingCopy)
{
	bool bRes = true;

	if(pFormula->getTypeSCM() == Formula::SCM_Git)
	{
		ConnectorGit connector(getEnv());
		bRes = connector.git_checkout(getConfigureVersion(), dirWorkingCopy);
	}
	if(pFormula->getTypeSCM() == Formula::SCM_Mercurial)
	{
		ConnectorMercurial connector(getEnv());
		bRes = connector.hg_checkout(getConfigureVersion(), dirWorkingCopy);
	}

	return bRes;
}

const QString& CommandPrepare::getConfigureVersion() const
{
	if(!m_szScmTagVersion.isEmpty()){
		return m_szScmTagVersion;
	}
	if(!m_szScmBranchVersion.isEmpty()){
		return m_szScmBranchVersion;
	}
	return getPackageVersion();
}
