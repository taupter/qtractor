// qtractorPluginCommand.cpp
//
/****************************************************************************
   Copyright (C) 2005-2010, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorAbout.h"
#include "qtractorPluginCommand.h"
#include "qtractorPluginForm.h"

#include "qtractorPluginListView.h"

#include "qtractorSession.h"

#include "qtractorMidiBuffer.h"


//----------------------------------------------------------------------
// class qtractorPluginCommand - implementation
//

// Constructor.
qtractorPluginCommand::qtractorPluginCommand ( const QString& sName,
	qtractorPlugin *pPlugin ) : qtractorCommand(sName)
{
	if (pPlugin)
		m_plugins.append(pPlugin);	

	setRefresh(false);
}


// Destructor.
qtractorPluginCommand::~qtractorPluginCommand (void)
{
	if (isAutoDelete())
		qDeleteAll(m_plugins);
	m_plugins.clear();
}


// Add new plugin(s) command methods.
bool qtractorPluginCommand::addPlugins (void)
{
	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return false;

	pSession->lock();

	// Add all listed plugins, in order...
	QListIterator<qtractorPlugin *> iter(m_plugins);
	while (iter.hasNext()) {
		qtractorPlugin *pPlugin = iter.next();
		qtractorPluginList *pPluginList = pPlugin->list();
		if (pPluginList)
			pPluginList->addPlugin(pPlugin);
	}
	// Avoid the disposal of the plugin reference(s).
	setAutoDelete(false);

	pSession->unlock();

	return true;
}


// Remove existing plugin(s) command methods.
bool qtractorPluginCommand::removePlugins (void)
{
	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return false;

	pSession->lock();

	// Unlink all listed plugins, in order...
	QListIterator<qtractorPlugin *> iter(m_plugins);
	iter.toBack();
	while (iter.hasPrevious()) {
		qtractorPlugin *pPlugin = iter.previous();
		qtractorPluginList *pPluginList = pPlugin->list();
		if (pPluginList)
			pPluginList->removePlugin(pPlugin);
	}
	// Allow the disposal of the plugin reference(s).
	setAutoDelete(true);

	pSession->unlock();

	return true;
}


//----------------------------------------------------------------------
// class qtractorAddPluginCommand - implementation
//

// Constructor.
qtractorAddPluginCommand::qtractorAddPluginCommand ( qtractorPlugin *pPlugin )
	: qtractorPluginCommand(QObject::tr("add plugin"), pPlugin)
{
}


// Plugin insertion command methods.
bool qtractorAddPluginCommand::redo (void)
{
	return addPlugins();
}

bool qtractorAddPluginCommand::undo (void)
{
	return removePlugins();
}


//----------------------------------------------------------------------
// class qtractorAddInsertPluginCommand - implementation
//

// Constructor.
qtractorAddInsertPluginCommand::qtractorAddInsertPluginCommand (
	qtractorPlugin *pPlugin ) : qtractorPluginCommand(
		QObject::tr("add insert"), pPlugin)
{
}


// Plugin insertion command methods.
bool qtractorAddInsertPluginCommand::redo (void)
{
	return addPlugins();
}

bool qtractorAddInsertPluginCommand::undo (void)
{
	return removePlugins();
}


//----------------------------------------------------------------------
// class qtractorRemovePluginCommand - implementation
//

// Constructor.
qtractorRemovePluginCommand::qtractorRemovePluginCommand (
	qtractorPlugin *pPlugin )
	: qtractorPluginCommand(QObject::tr("remove plugin"), pPlugin)
{
}


// Plugin-removal command methods.
bool qtractorRemovePluginCommand::redo (void)
{
	return removePlugins();
}

bool qtractorRemovePluginCommand::undo (void)
{
	return addPlugins();
}


//----------------------------------------------------------------------
// class qtractorInsertPluginCommand - implementation
//

// Constructor.
qtractorInsertPluginCommand::qtractorInsertPluginCommand (
	const QString& sName, qtractorPlugin *pPlugin,
	qtractorPlugin *pNextPlugin ) : qtractorPluginCommand(sName, pPlugin)
{
	m_pNextPlugin = pNextPlugin;
}


// Plugin-insert command methods.
bool qtractorInsertPluginCommand::redo (void)
{
	qtractorPlugin *pPlugin = plugins().first();
	if (pPlugin == NULL)
		return false;

	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return false;

	// Save the previous track alright...
	qtractorPluginList *pPluginList = pPlugin->list();
	if (pPluginList == NULL)
		return false;

	pSession->lock();

	qtractorPlugin *pNextPlugin = pPlugin->next();

	// Insert it...
	pPluginList->insertPlugin(pPlugin, m_pNextPlugin);

	// Swap it nice, finally.
	m_pNextPlugin = pNextPlugin;

	// Whether to allow the disposal of the plugin reference.
	setAutoDelete(false);

	pSession->unlock();

	return true;
}

bool qtractorInsertPluginCommand::undo (void)
{
	qtractorPlugin *pPlugin = plugins().first();
	if (pPlugin == NULL)
		return false;

	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return false;

	// Save the previous track alright...
	qtractorPluginList *pPluginList = pPlugin->list();
	if (pPluginList == NULL)
		return false;

	pSession->lock();

	qtractorPlugin *pNextPlugin = pPlugin->next();

	// Insert it...
	pPluginList->removePlugin(pPlugin);

	// Swap it nice, finally.
	m_pNextPlugin = pNextPlugin;

	// Whether to allow the disposal of the plugin reference.
	setAutoDelete(true);

	pSession->unlock();

	return true;
}


//----------------------------------------------------------------------
// class qtractorMovePluginCommand - implementation
//

// Constructor.
qtractorMovePluginCommand::qtractorMovePluginCommand (
	qtractorPlugin *pPlugin, qtractorPlugin *pNextPlugin,
		qtractorPluginList *pPluginList )
	: qtractorInsertPluginCommand(QObject::tr("move plugin"),
		pPlugin, pNextPlugin)
{
	m_pPluginList = pPluginList;
}


// Plugin-move command methods.
bool qtractorMovePluginCommand::redo (void)
{
	qtractorPlugin *pPlugin = plugins().first();
	if (pPlugin == NULL)
		return false;

	if (m_pPluginList == NULL)
		return false;

	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return false;

	pSession->lock();

	// Save the previous track alright...
	qtractorPlugin *pNextPlugin = pPlugin->next();
	qtractorPluginList *pPluginList = pPlugin->list();

	// Move it...
	m_pPluginList->movePlugin(pPlugin, nextPlugin());

	// Swap it nice, finally.
	m_pPluginList = pPluginList;
	setNextPlugin(pNextPlugin);

	pSession->unlock();

	return true;
}

bool qtractorMovePluginCommand::undo (void)
{
	// As we swap the prev/state this is non-idempotent.
	return redo();
}


//----------------------------------------------------------------------
// class qtractorActivatePluginCommand - implementation
//

// Constructor.
qtractorActivatePluginCommand::qtractorActivatePluginCommand (
	qtractorPlugin *pPlugin, bool bActivated )
	: qtractorPluginCommand(QObject::tr("activate plugin"), pPlugin)
{
	m_bActivated = bActivated;
}


// Plugin-activate command methods.
bool qtractorActivatePluginCommand::redo (void)
{
	// Save the toggled state alright...
	bool bActivated = !m_bActivated;

	QListIterator<qtractorPlugin *> iter(plugins());
	while (iter.hasNext())
		iter.next()->setActivated(m_bActivated);

	// Swap it nice, finally.
	m_bActivated = bActivated;

	return true;
}

bool qtractorActivatePluginCommand::undo (void)
{
	// As we toggle the prev/state this is non-idempotent.
	return redo();
}


//----------------------------------------------------------------------
// class qtractorPresetPluginCommand - implementation
//

// Constructor.
qtractorPresetPluginCommand::qtractorPresetPluginCommand (
	qtractorPlugin *pPlugin, const QStringList& vlist )
	: qtractorPluginCommand(QObject::tr("preset plugin"), pPlugin)
{
	m_vlist = vlist;
}


// Plugin-preset command methods.
bool qtractorPresetPluginCommand::redo (void)
{
	qtractorPlugin *pPlugin = plugins().first();
	if (pPlugin == NULL)
		return false;

	// Save the current toggled state alright...
	QStringList vlist = pPlugin->valueList();
	pPlugin->setValueList(m_vlist);
	pPlugin->realizeValues();
	// Swap it nice, finally.
	m_vlist = vlist;

	// Update the form, showing it up as necessary...
	pPlugin->form()->refresh();

	return true;
}

bool qtractorPresetPluginCommand::undo (void)
{
	// As we swap the prev/state this is non-idempotent.
	return redo();
}


//----------------------------------------------------------------------
// class qtractorResetPluginCommand - implementation
//

// Constructor.
qtractorResetPluginCommand::qtractorResetPluginCommand (
	qtractorPlugin *pPlugin )
	: qtractorPluginCommand(QObject::tr("reset plugin"), pPlugin)
{
	m_bReset = false;
}


// Plugin-reset command methods.
bool qtractorResetPluginCommand::redo (void)
{
	qtractorPlugin *pPlugin = plugins().first();
	if (pPlugin == NULL)
		return false;

	// Toggle/swap it nice...
	m_bReset = !m_bReset;
	if (m_bReset) {
		m_vlist = pPlugin->valueList();
		pPlugin->reset();
	} else {
		pPlugin->setValueList(m_vlist);
		pPlugin->realizeValues();
	}

	// Update the form, showing it up as necessary...
	pPlugin->form()->refresh();

	return true;
}


bool qtractorResetPluginCommand::undo (void)
{
	// As we swap the prev/state this is non-idempotent.
	return redo();
}


//----------------------------------------------------------------------
// class qtractorPluginParamCommand - implementation
//

// Constructor.
qtractorPluginParamCommand::qtractorPluginParamCommand (
	qtractorPluginParam *pParam, float fValue, bool bUpdate )
	: qtractorCommand(QString(pParam->name()).toLower()),
		m_pParam(pParam), m_fValue(fValue), m_bUpdate(bUpdate),
		m_fPrevValue(pParam->prevValue())
{
	setRefresh(false);

	// Try replacing an previously equivalent command...
	static qtractorPluginParamCommand *s_pPrevParamCommand = NULL;
	if (s_pPrevParamCommand) {
		qtractorSession *pSession = qtractorSession::getInstance();
		qtractorCommand *pLastCommand
			= (pSession->commands())->lastCommand();
		qtractorCommand *pPrevCommand
			= static_cast<qtractorCommand *> (s_pPrevParamCommand);
		if (pPrevCommand == pLastCommand
			&& s_pPrevParamCommand->param() == pParam) {
			qtractorPluginParamCommand *pLastParamCommand
				= static_cast<qtractorPluginParamCommand *> (pLastCommand);
			if (pLastParamCommand) {
				// Equivalence means same (sign) direction too...
				float fPrevValue = pLastParamCommand->prevValue();
				float fLastValue = pLastParamCommand->value();
				int   iPrevSign  = (fPrevValue > fLastValue ? +1 : -1);
				int   iCurrSign  = (fPrevValue < m_fValue   ? +1 : -1); 
				if (iPrevSign == iCurrSign) {
					m_fPrevValue = fLastValue;
					(pSession->commands())->removeLastCommand();
				}
			}
		}
	}
	s_pPrevParamCommand = this;
}


// Plugin-reset command methods.
bool qtractorPluginParamCommand::redo (void)
{
	qtractorPlugin *pPlugin = m_pParam->plugin();
	if (pPlugin == NULL)
		return false;

	// Set plugin parameter value...
	float fValue = m_fPrevValue;

	m_pParam->setValue(m_fValue, m_bUpdate);

	// Set undo value...
	m_fPrevValue = m_fValue;
	m_fValue     = fValue;
	m_bUpdate    = true;

	// Update the form, showing it up as necessary...
	(pPlugin->form())->updateParamWidget(m_pParam->index());
	// Update any GUI editor...
	// pPlugin->idleEditor();

	return true;
}


bool qtractorPluginParamCommand::undo (void)
{
	// As we swap the prev/value this is non-idempotent.
	return redo();
}


//----------------------------------------------------------------------
// class qtractorAudioOutputBusCommand - declaration.
//

// Constructor.
qtractorAudioOutputBusCommand::qtractorAudioOutputBusCommand (
	qtractorMidiManager *pMidiManager, bool bAudioOutputBus )
	: qtractorCommand(QObject::tr("dedicated audio outputs")),
		m_pMidiManager(pMidiManager),
		m_bAudioOutputBus(bAudioOutputBus)
{
}


// Plugin audio ouput bus command methods.
bool qtractorAudioOutputBusCommand::redo (void)
{
	if (m_pMidiManager == NULL)
		return false;

	bool bAudioOutputBus = m_pMidiManager->isAudioOutputBus();
	m_pMidiManager->setAudioOutputBus(m_bAudioOutputBus);
	m_bAudioOutputBus = bAudioOutputBus;

	return true;
}

bool qtractorAudioOutputBusCommand::undo (void)
{
	return redo();
}


// end of qtractorPluginCommand.cpp
