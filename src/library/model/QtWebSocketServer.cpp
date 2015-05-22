/*

  INScore Project

  Copyright (C) 2015  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  research@grame.fr

*/

#include <QWebSocket>
#include <string>
#include <iostream>
#include <sstream>

#include "QtWebSocketServer.h"
#include "IWebSocket.h"
#include "VObjectView.h"
#include "VSceneView.h"
#include "abstractdata.h"
#include "ITLparser.h"

using namespace std;

namespace inscore {

//-------------------------------------------------------------------------------
QtWebSocketServer::QtWebSocketServer(int frequency, VObjectView *view, TJSEngine* engine, TLua* lua) :
	QWebSocketServer(QStringLiteral("WebSocketServer"), QWebSocketServer::NonSecureMode),
	fScreenVersion(0), fView (view), fFrequency(frequency), fJsEngine(engine), fLua(lua)
{
	connect(&fTimer, SIGNAL(timeout()),this, SLOT(timeTask()));
}

//-------------------------------------------------------------------------------
QtWebSocketServer::~QtWebSocketServer()
{
	stop();
}

//-------------------------------------------------------------------------------
bool QtWebSocketServer::start(int port)
{
	stop();										// stop the server, in case it's already running

	if (listen(QHostAddress::Any, port)) {		// and start listening in port 'port'
		connect(this, &QWebSocketServer::newConnection, this, &QtWebSocketServer::onNewConnection);
		connect(this, &QWebSocketServer::closed, this, &QtWebSocketServer::closed);
		fTimer.start(fFrequency);
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::stop()
{
	fTimer.stop ();
	close();										// close the server
	qDeleteAll(fClients.begin(), fClients.end());	// and delete all the clients.
	fClients.clear();
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::onNewConnection()
{
	QWebSocket *pSocket = nextPendingConnection();
	// Connect client to action.
	connect(pSocket, &QWebSocket::disconnected, this, &QtWebSocketServer::socketDisconnected);
	connect(pSocket, &QWebSocket::textMessageReceived, this, &QtWebSocketServer::processTextMessage);
	fClients << pSocket;		// add the client to the clients list
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::setFrequency(int frequency)
{
	fFrequency = frequency;
	fTimer.stop();
	fTimer.start(fFrequency);
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::socketDisconnected()
{
	// Retrieve the client
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient) {
		// Remove the client in the list
		fClients.removeAll(pClient);
		pClient->deleteLater();
	}
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::timeTask()
{
	if(fView->isNewVersion(fScreenVersion)) {		// check for view updates

		// The screen have been updated, send notifications to all the clients
		QList<QWebSocket *>::iterator i;
		for (i = fClients.begin(); i != fClients.end(); ++i)  {
			(*i)->sendTextMessage("Screen updated");
		}
	}
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::processTextMessage(QString message)
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient) {
		// Verify message and get back image and send it
		if(message == IWebSocket::kGetImgMsg) {
			getImage(pClient);
		} else
		if(message.startsWith(IWebSocket::kPostMsg)) {
			postCommand(message);
		} else
		if(message.startsWith(IWebSocket::kClickMsg)) {
			mouseClick(message);
		} else
			pClient->sendTextMessage("Error : unknown request.");
	}
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::getImage(QWebSocket *pClient)
{
	AbstractData data = fView->getImage("PNG");
	QByteArray bArray = QByteArray::fromRawData(data.data, data.size);
	pClient->sendBinaryMessage(bArray);
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::postCommand(QString &commands)
{
	// Remove request name (ie "post=")
	commands = commands.remove(0, 5);
	stringstream stream;
	stream.str(commands.toStdString());
	ITLparser p (&stream, 0, fJsEngine, fLua);
	SIMessageList msgs = p.parse();
	msgs->send();
}

//-------------------------------------------------------------------------------
void QtWebSocketServer::mouseClick(QString &coordinates)
{
	// Remove request name (ie "click=")
	coordinates = coordinates.remove(0, 6);
	QStringList coord = coordinates.split(",", QString::SkipEmptyParts);
	if(coord.size() == 2) {
		int x = coord[0].toInt();
		int y = coord[1].toInt();

		// Get item from coordinate in pixel.
		QGraphicsItem * item = getItem(x, y);

		if(item) {
			// Create and send a mouse event to the item
			sendEvent(item, QEvent::GraphicsSceneMousePress);
		}
	}
}

//-------------------------------------------------------------------------------.
QGraphicsItem * QtWebSocketServer::getItem(int x, int y)
{
	VSceneView * sceneView = dynamic_cast<VSceneView *>(fView);
	if(sceneView) {
		QGraphicsView* view = sceneView->view();

		// get item at position (x and y in pixel coordinate)
		return view->itemAt(x, y);
	}
	return 0;
}

//-------------------------------------------------------------------------------.
void QtWebSocketServer::sendEvent(QGraphicsItem * item, QEvent::Type eventType)
{
	VSceneView * sceneView = dynamic_cast<VSceneView *>(fView);
	// Create an event
	QGraphicsSceneMouseEvent event(eventType);
	// Coordinate of the click in item coordinate
	QPointF point(0, 0);
	event.setPos(point);
	event.setButton(Qt::LeftButton);
	event.setButtons(Qt::LeftButton);

	if(!item->isEnabled())
		item->setEnabled(true);

	QGraphicsScene * scene = sceneView->scene();
	scene->sendEvent(item, &event);
}
}