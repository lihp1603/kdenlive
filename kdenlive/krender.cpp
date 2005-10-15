/***************************************************************************
                        krender.cpp  -  description
                           -------------------
  begin                : Fri Nov 22 2002
  copyright            : (C) 2002 by Jason Wood
  email                : jasonwood@blueyonder.co.uk
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kio/netaccess.h>
#include <kdebug.h>
#include <klocale.h>

#include "krender.h"

#include "avformatdescbool.h"
#include "avformatdesclist.h"
#include "avformatdesccontainer.h"
#include "avformatdesccodeclist.h"
#include "avformatdesccodec.h"
#include <mlt++/Mlt.h>
#include <iostream>
#include "effectparamdesc.h"

KRender::KRender( const QString &rendererName, KURL appPath, unsigned int port, QObject *parent, const char *name ) :
		QObject( parent, name ),
		QXmlDefaultHandler(),
		m_name( rendererName ),
		m_renderName( "unknown" ),
		m_renderVersion( "unknown" ),
		m_appPathInvalid( false ),
		m_fileFormat( 0 ),
		m_desccodeclist( 0 ),
		m_codec( 0 ),
		m_effect( 0 ),
		m_playSpeed( 0.0 ),
		m_parameter(0)
{
	startTimer( 1000 );
	m_parsing = false;
	m_setSceneListPending = false;

	m_fileFormats.setAutoDelete( true );
	m_codeclist.setAutoDelete( true );
	m_effectList.setAutoDelete( true );

	m_xmlReader.setContentHandler( this );

	connect( &m_socket, SIGNAL( error( int ) ), this, SLOT( error( int ) ) );
	connect( &m_socket, SIGNAL( connected() ), this, SLOT( slotConnected() ) );
	connect( &m_socket, SIGNAL( connectionClosed() ), this, SLOT( slotDisconnected() ) );
	connect( &m_socket, SIGNAL( readyRead() ), this, SLOT( readData() ) );

	connect( &m_process, SIGNAL( processExited( KProcess * ) ), this, SLOT( processExited() ) );
	connect( &m_process, SIGNAL( receivedStdout( KProcess *, char *, int ) ), this, SLOT( slotReadStdout( KProcess *, char *, int ) ) );
	connect( &m_process, SIGNAL( receivedStderr( KProcess *, char *, int ) ), this, SLOT( slotReadStderr( KProcess *, char *, int ) ) );

	m_portNum = port;
	m_appPath = appPath;
	consumer=0;
	openMlt();
}

KRender::~KRender()
{
	closeMlt();
	killTimers();
	quit();
}

/** Recieves timer events */
void KRender::timerEvent( QTimerEvent *event )
{
	//if ( m_socket.state() == QSocket::Idle ) {
		//if ( !m_process.isRunning() ) {
			//if(m_refCount==1){
			if (consumer==0){
				launchProcess();
				emit slotConnected();
			}
			//}
		//} else {
		//	m_socket.connectToHost( "127.0.0.1", m_portNum );
		//}
	//}
}

/** Catches errors from the socket. */
void KRender::error( int error )
{
	switch ( error ) {
		case QSocket::ErrConnectionRefused :
		emit renderWarning( m_name, "Connection Refused" );
		break;
		case QSocket::ErrHostNotFound :
		emit renderWarning( m_name, "Host Not Found" );
		break;
		case QSocket::ErrSocketRead :
		emit renderWarning( m_name, "Error Reading Socket" );
		break;
	}
}

/** Called when we have connected to the renderer. */
void KRender::slotConnected()
{
	getCapabilities();

	emit renderDebug( m_name, "Connected on port " + QString::number( m_socket.port() ) +
	                  " to host on port " + QString::number( m_socket.peerPort() ) );
	emit initialised();
	emit connected();
}

/** Called when we have disconnected from the renderer. */
void KRender::slotDisconnected()
{
	emit renderWarning( m_name, "Disconnected" );

	emit disconnected();
}

/** Called when some data has been recieved by the socket, reads the data and processes it. */
void KRender::readData()
{
}

/** Sends an XML command to the renderer. */
void KRender::sendCommand( QDomDocument command )
{
}

/** Generates the quit command */
void KRender::quit()
{
}

/** Called if the rendering process has exited. */
void KRender::processExited()
{
	emit renderWarning( m_name, "Render Process Exited" );
}

/** Launches a renderer process. */
void KRender::launchProcess()
{
//   Removed some obsolete code that was responsible for artsd launch,
//   but wasn't used anymore.
	emit renderWarning( m_name, "Render should start" );
}

void KRender::slotReadStdout( KProcess *proc, char *buffer, int buflen )
{
	QString mess;
	mess.setLatin1( buffer, buflen );
	emit recievedStdout( m_name, mess );
}

void KRender::slotReadStderr( KProcess *proc, char *buffer, int buflen )
{
	QString mess;
	mess.setLatin1( buffer, buflen );
	emit recievedStderr( m_name, mess );
}

/** Returns a list of all available file formats in this renderer. */
QPtrList<AVFileFormatDesc> &KRender::fileFormats()
{
	return m_fileFormats;
}



void KRender::openMlt(){
	m_refCount++;
	if (m_refCount==1){
		Mlt::Factory::init();
		m_mltMiracle=new Mlt::Miracle("miracle",5250);
		m_mltMiracle->start();
		std::cout << "Mlt inited" << std::endl;
	}
}

void KRender::closeMlt(){
	m_refCount--;
	if (m_refCount==1){
		//m_mltMiracle->wait_for_shutdown();
		//delete(m_mltMiracle);
	}
}


/** Wraps the VEML command of the same name; requests that the renderer
should create a video window. If show is true, then the window should be
displayed, otherwise it should be hidden. KRender will emit the signal
replyCreateVideoXWindow() once the renderer has replied. */


static void consumer_frame_show (mlt_consumer sdl, KRender* self,mlt_frame frame_ptr){
	//std::cout << frame_ptr << std::endl;
}
void my_lock(){
	mutex.lock();
}
void my_unlock(){
	mutex.unlock();
}
void KRender::createVideoXWindow( bool show ,WId winid)
{
	
	
	consumer=new Mlt::Consumer("sdl_preview:352x288");
	consumer->listen("consumer-frame-show",this,(mlt_listener)consumer_frame_show);
	consumer->set ("app_locked",1);
	consumer->set("app_lock",(void*)my_lock,0);
	consumer->set("app_unlock",(void*)my_unlock,0);
	setenv("SDL_WINDOWID",QString::number(winid),1);
	std::cout << "winid: << "<< winid << std::endl;
	consumer->set("resize",1);
	consumer->set("progressiv",1);
	
	/*std::cout << m_refCount << std::endl;
	if (m_refCount==5){*/
	
	Mlt::Producer *pr=new Mlt::Producer("noise");
	
	consumer->connect(*pr);
	consumer->start();
}

/** Wraps the VEML command of the same name; Seeks the renderer clip to the given time. */
void KRender::seek( GenTime time )
{
	sendSeekCommand( time );
	emit positionChanged( time );
}

void KRender::getFileProperties( KURL url )
{
	if ( !rendererOk() ) {
		emit replyErrorGetFileProperties( url.path(), i18n( "The renderer is unavailable, the file properties cannot be determined." ) );
	} else {
		QDomDocument doc;
		QDomElement elem = doc.createElement( "getFileProperties" );

		elem.setAttribute( "filename", url.path() );
		doc.appendChild( elem );

		sendCommand( doc );
	}
}

/** Wraps the VEML command of the same name. Sets the current scene list to
be list. */
void KRender::setSceneList( QDomDocument list )
{
	m_sceneList = list;
	m_setSceneListPending = true;
}

/** Wraps the VEML command of the same name - sends a <ping> command to the server, which
should reply with a <pong> - let's us determine the round-trip latency of the connection. */
void KRender::ping( QString &ID )
{
	QDomDocument doc;
	QDomElement elem = doc.createElement( "ping" );
	elem.setAttribute( "id", ID );
	doc.appendChild( elem );
	sendCommand( doc );
}

void KRender::play( double speed )
{
	m_playSpeed = speed;
	if ( m_setSceneListPending ) {
		sendSetSceneListCommand( m_sceneList );
	}
	QDomDocument doc;
	QDomElement elem = doc.createElement( "play" );
	elem.setAttribute( "speed", speed );
	doc.appendChild( elem );
	sendCommand( doc );
}

void KRender::play(double speed, const GenTime &startTime)
{
	m_playSpeed = speed;
	if(m_setSceneListPending) {
		sendSetSceneListCommand(m_sceneList);
	}
	QDomDocument doc;
	QDomElement elem = doc.createElement("play");
	elem.setAttribute("speed", speed);
	elem.setAttribute("start", startTime.seconds());
	doc.appendChild(elem);
	sendCommand(doc);
}

void KRender::play( double speed, const GenTime &startTime, const GenTime &stopTime )
{
	m_playSpeed = speed;

	if ( m_setSceneListPending ) {
		sendSetSceneListCommand( m_sceneList );
	}
	QDomDocument doc;
	QDomElement elem = doc.createElement( "play" );
	elem.setAttribute( "speed", speed );
	elem.setAttribute( "start", startTime.seconds() );
	elem.setAttribute( "stop", stopTime.seconds() );
	doc.appendChild( elem );
	sendCommand( doc );
}

void KRender::render( const KURL &url )
{
	if ( m_setSceneListPending ) {
		sendSetSceneListCommand( m_sceneList );
	}
	QDomDocument doc;
	QDomElement elem = doc.createElement( "render" );
	elem.setAttribute( "filename", url.path() );
	doc.appendChild( elem );
	sendCommand( doc );
}

void KRender::sendSeekCommand( GenTime time )
{
	if ( m_setSceneListPending ) {
		sendSetSceneListCommand( m_sceneList );
	}

	QDomDocument doc;
	QDomElement elem = doc.createElement( "seek" );
	elem.setAttribute( "time", QString::number( time.seconds() ) );
	doc.appendChild( elem );
	sendCommand( doc );

	m_seekPosition = time;
}

void KRender::sendSetSceneListCommand( const QDomDocument &list )
{
	m_setSceneListPending = false;

	QDomDocument doc;
	QDomElement elem = doc.createElement( "setSceneList" );
	elem.appendChild( doc.importNode( list.documentElement(), true ) );
	doc.appendChild( elem );
	sendCommand( doc );
}

void KRender::getCapabilities()
{
	QDomDocument doc;
	QDomElement elem = doc.createElement( "getCapabilities" );
	doc.appendChild( elem );
	sendCommand( doc );
}

void KRender::pushIgnore()
{
	StackValue val;
	val.element = "ignore";
	val.funcStartElement = 0;
	val.funcEndElement = 0;
	m_parseStack.push( val );
}

// Pushes a value onto the stack.
void KRender::pushStack( QString element,
                         bool ( KRender::*funcStartElement ) ( const QString & localName, const QString & qName, const QXmlAttributes & atts ),
                         bool ( KRender::*funcEndElement ) ( const QString & localName, const QString & qName ) )
{
	StackValue val;
	val.element = element;
	val.funcStartElement = funcStartElement;
	val.funcEndElement = funcEndElement;
	m_parseStack.push( val );
}

/** Returns the codec with the given name */
AVFormatDescCodec * KRender::findCodec( const QString &name )
{
	QPtrListIterator<AVFormatDescCodec> itt( m_codeclist );

	while ( itt.current() ) {
		emit renderWarning( m_name, "Comparing " + name + " with " + itt.current() ->name() );
		if ( name == itt.current() ->name() ) return itt.current();
		++itt;
	}
	return 0;
}

/** Returns the effect list. */
const EffectDescriptionList &KRender::effectList() const
{
	return m_effectList;
}

/** Sets the renderer version for this renderer. */
void KRender::setVersion( QString version )
{
	m_version = version;
}

/** Returns the renderer version. */
QString KRender::version()
{
	return m_version;
}

/** Sets the description of this renderer to desc. */
void KRender::setDescription( const QString &description )
{
	m_description = description;
}

/** Returns the description of this renderer */
QString KRender::description()
{
	return m_description;
}








/** Occurs upon starting to parse an XML document */
bool KRender::startDocument()
{
	//  emit renderDebug(m_name, "Starting to parse document");
	return true;
}

/** Occurs upon finishing reading an XML document */
bool KRender::endDocument()
{
	//  emit renderDebug(m_name, "Finishing parsing document");
	return true;
}

/** Called when the xml parser encounters an opening element */
bool KRender::startElement( const QString &nameSpace, const QString & localName,
                            const QString & qName, const QXmlAttributes & atts )
{
	StackValue val = m_parseStack.top();
	if ( val.funcStartElement == NULL ) {
		emit renderDebug( m_name, "Ignoring tag " + localName );
		pushIgnore();
		return true;
	}
	return ( this->*val.funcStartElement ) ( localName, qName, atts );
}

/** Called when the xml parser encounters a closing tag */
bool KRender::endElement ( const QString &nameSpace, const QString & localName, const QString & qName )
{
	StackValue val = m_parseStack.pop();
	if ( val.funcEndElement == NULL ) return true;
	return ( this->*val.funcEndElement ) ( localName, qName );
}

bool KRender::characters( const QString &ch )
{
	m_characterBuffer += ch;
	return true;
}


bool KRender::rendererOk()
{
	//if ( m_appPathInvalid ) return false;

	return true;
}

double KRender::playSpeed()
{
	return m_playSpeed;
}

const GenTime &KRender::seekPosition() const
{
	return m_seekPosition;
}

void KRender::sendDebugVemlCommand(const QString &name)
{
	if ( m_socket.state() == QSocket::Connected ) {
		kdWarning() << "Sending debug command " << name << endl;
		QCString str = (name + "\n\n").latin1();
		m_socket.writeBlock( str, strlen( str ) );
	} else {
		emit renderWarning( m_name, "Socket not connected, not sending Command " +
		                    name );
	}
}

const QString &KRender::rendererName() const
{
	return m_name;
}

void KRender::setCapture()
{
	QDomDocument doc;
	QDomElement elem = doc.createElement( "setCapture" );
	doc.appendChild( elem );

	sendCommand( doc );
}
