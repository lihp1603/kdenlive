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
#include "krender.h"

#include "avformatdescbool.h"
#include "avformatdesclist.h"
#include "avformatdesccontainer.h"
#include "avformatdesccodeclist.h"
#include "avformatdesccodec.h"

KRender::KRender(const QString &rendererName, KURL appPath, unsigned int port, QObject *parent, const char *name ) :
																				QObject(parent, name),
																				QXmlDefaultHandler(),
                                        m_name(rendererName),
                                        m_renderName("unknown"),
                                        m_renderVersion("unknown"),
                                        m_appPathInvalid(false),
                                        m_fileFormat(0),
                                        m_desccodeclist(0),
                                        m_codec(0),
                                        m_effect(0)
{
	startTimer(1000);
	m_parsing = false;
	m_seekPending = false;
	m_nextSeek = -1.0;
	m_setSceneListPending = false;

  m_codeclist.setAutoDelete(true);
  m_effectList.setAutoDelete(true);  

	m_xmlReader.setContentHandler(this);

	connect(&m_socket, SIGNAL(error(int)), this, SLOT(error(int)));
	connect(&m_socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(&m_socket, SIGNAL(connectionClosed()), this, SLOT(slotDisconnected()));
	connect(&m_socket, SIGNAL(readyRead()), this, SLOT(readData()));

	connect(&m_process, SIGNAL(processExited(KProcess *)), this, SLOT(processExited()));
  connect(&m_process, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotReadStdout(KProcess *, char *, int)));
  connect(&m_process, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(slotReadStderr(KProcess *, char *, int)));

	m_portNum = port;
  m_appPath = appPath;
}

KRender::~KRender()
{
	killTimers();
	quit();
}

/** Recieves timer events */
void KRender::timerEvent(QTimerEvent *event)
{
  if(m_socket.state() == QSocket::Idle) {
    if(!m_process.isRunning()) {
     	launchProcess();
    } else {
     	m_socket.connectToHost("127.0.0.1", m_portNum);
    }
  }
}

/** Catches errors from the socket. */
void KRender::error(int error)
{
	switch(error) {
		case QSocket::ErrConnectionRefused :
              emit recievedInfo(m_name, "Connection Refused");
							break;
		case QSocket::ErrHostNotFound :
              emit recievedInfo(m_name, "Host Not Found");
							break;
		case QSocket::ErrSocketRead :
              emit recievedInfo(m_name, "Error Reading Socket");
							break;
	}
}

/** Called when we have connected to the renderer. */
void KRender::slotConnected()
{
  getCapabilities();

  emit recievedInfo(m_name, "Connected on port " + QString::number(m_socket.port()) + " to host on port " + QString::number(m_socket.peerPort()));
	emit initialised();
  emit connected();
}

/** Called when we have disconnected from the renderer. */
void KRender::slotDisconnected()
{
  emit recievedInfo(m_name, "Disconnected");

  emit disconnected();
}

/** Called when some data has been recieved by the socket, reads the data and processes it. */
void KRender::readData()
{
  m_buffer += QString(m_socket.readAll());
  int pos;

  while((pos = m_buffer.find("\n\n")) != -1) {
    QString temp = m_buffer.left(pos+1);
    m_buffer = m_buffer.right(m_buffer.length() - pos);
    while(m_buffer.left(1) == "\n") m_buffer = m_buffer.right(m_buffer.length() -1);

    QXmlInputSource source;
    source.setData(temp);

    m_parseStack.clear();
    StackValue value;
    value.element="root";
    value.funcStartElement = &KRender::topLevelStartElement;
    value.funcEndElement = 0;
    m_parseStack.push(value);

    if(!m_xmlReader.parse(&source, false)) {
      emit recievedInfo(m_name, "Parse Failed on " + temp);
    } else {
      emit recievedInfo(m_name, "Parse successfull");
    }
  }
}

/** Sends an XML command to the renderer. */
void KRender::sendCommand(QDomDocument command)
{
//	kdDebug() << "Sending Command " << command.toString() << endl;
	QCString str = command.toCString() + "\n";
	m_socket.writeBlock(str, strlen(str));
}

/** Generates the quit command */
void KRender::quit()
{
	QDomDocument doc;
	doc.appendChild(doc.createElement("quit"));
	sendCommand(doc);
}

/** Called if the rendering process has exited. */
void KRender::processExited()
{
  emit recievedInfo(m_name, "Render Process Exited");
}

/** Launches a renderer process. */
void KRender::launchProcess()
{
  if(m_appPathInvalid) return;

  if(!KIO::NetAccess::exists(m_appPath)) {
    emit recievedInfo(m_name, "application '" + m_appPath.path() + "' does not exist");
    m_appPathInvalid = true;
    return;
  }
	m_process.clearArguments();
	m_process.setExecutable("artsdsp");
  m_process << m_appPath.path();
  m_process << "-d";
  m_process << "-p" << QString::number(m_portNum);

  emit recievedInfo(m_name, "Launching Process " + m_appPath.path() + " as server on port " + QString::number(m_portNum));
	if(m_process.start(KProcess::NotifyOnExit, KProcess::AllOutput)) {
    emit recievedInfo(m_name, "Process launching successfully, pid = " + QString::number(m_process.pid()));
    emit recievedInfo(m_name, "Connecting to server on port " + QString::number(m_portNum));
		m_socket.connectToHost("127.0.0.1", m_portNum);
	} else {
    emit recievedInfo(m_name, "Could not start process");
	}
}

void KRender::slotReadStdout(KProcess *proc, char *buffer, int buflen)
{
  QString mess;
  mess.setLatin1(buffer, buflen);
  emit recievedStdout(m_name, mess);
}

void KRender::slotReadStderr(KProcess *proc, char *buffer, int buflen)
{
  QString mess;
  mess.setLatin1(buffer, buflen);
  emit recievedStderr(m_name, mess);
}

/** Returns a list of all available file formats in this renderer. */
QPtrList<AVFileFormatDesc> &KRender::fileFormats()
{
  return m_fileFormats;
}





/** Wraps the VEML command of the same name; requests that the renderer
should create a video window. If show is true, then the window should be
displayed, otherwise it should be hidden. KRender will emit the signal
replyCreateVideoXWindow() once the renderer has replied. */
void KRender::createVideoXWindow(bool show)
{
	QDomDocument doc;
	QDomElement elem = doc.createElement("createVideoXWindow");
	elem.setAttribute("show", show ? "true" : "false");
	elem.setAttribute("format", "rgb");
	doc.appendChild(elem);

	sendCommand(doc);
}

/** Wraps the VEML command of the same name; Seeks the renderer clip to the given time. */
void KRender::seek(GenTime time)
{
    sendSeekCommand(time);
/*  if(m_seekPending) {
    m_nextSeek = time;
  } else {
    sendSeekCommand(time);
    m_seekPending = true;
  }*/
}

void KRender::getFileProperties(KURL url)
{
	QDomDocument doc;
	QDomElement elem = doc.createElement("getFileProperties");

	elem.setAttribute("filename", url.path());
	doc.appendChild(elem);

	sendCommand(doc);
}

/** Wraps the VEML command of the same name. Sets the current scene list to
be list. */
void KRender::setSceneList(QDomDocument list)
{
	m_sceneList = list;
	m_setSceneListPending = true;
}

/** Wraps the VEML command of the same name - sends a <ping> command to the server, which
should reply with a <pong> - let's us determine the round-trip latency of the connection. */
void KRender::ping(QString &ID)
{
	QDomDocument doc;
	QDomElement elem = doc.createElement("ping");
	elem.setAttribute("id", ID);
	doc.appendChild(elem);
	sendCommand(doc);
}

void KRender::play(double speed)
{
	if(m_setSceneListPending) {
		sendSetSceneListCommand(m_sceneList);
	}
	QDomDocument doc;
	QDomElement elem = doc.createElement("play");
	elem.setAttribute("speed", speed);
	doc.appendChild(elem);
	sendCommand(doc);
}

void KRender::render(const KURL &url)
{
	if(m_setSceneListPending) {
		sendSetSceneListCommand(m_sceneList);
	}
	QDomDocument doc;
	QDomElement elem = doc.createElement("render");
	elem.setAttribute("filename", url.path());
	doc.appendChild(elem);
	sendCommand(doc);
}

void KRender::sendSeekCommand(GenTime time)
{
	if(m_setSceneListPending) {
		sendSetSceneListCommand(m_sceneList);
	}

 	QDomDocument doc;
 	QDomElement elem = doc.createElement("seek");
 	elem.setAttribute("time", QString::number(time.seconds()));
 	doc.appendChild(elem);
	sendCommand(doc);
}

void KRender::sendSetSceneListCommand(const QDomDocument &list)
{
	m_setSceneListPending = false;

	QDomDocument doc;
	QDomElement elem = doc.createElement("setSceneList");
	elem.appendChild(doc.importNode(list.documentElement(), true));
	doc.appendChild(elem);
	sendCommand(doc);
}

void KRender::getCapabilities()
{
  QDomDocument doc;
  QDomElement elem = doc.createElement("getCapabilities");
  doc.appendChild(elem);
  sendCommand(doc);
}

void KRender::pushIgnore()
{
  StackValue val;
  val.element = "ignore";
	val.funcStartElement = 0;
	val.funcEndElement = 0;
  m_parseStack.push(val);
}

/** Returns the codec with the given name */
AVFormatDescCodec * KRender::findCodec(const QString &name)
{
  QPtrListIterator<AVFormatDescCodec> itt(m_codeclist);

  while(itt.current()) {
    emit recievedInfo(m_name, "Comparing " + name + " with " + itt.current()->name());
    if(name == itt.current()->name()) return itt.current();
    ++itt;
  }
  return 0;
}









/** Occurs upon starting to parse an XML document */
bool KRender::startDocument()
{
  emit recievedInfo(m_name, "Starting to parse document");
	return true;
}

/** Occurs upon finishing reading an XML document */
bool KRender::endDocument()
{
  emit recievedInfo(m_name, "Finishing parsing document");
	return true;
}

/** Called when the xml parser encounters an opening element */
bool KRender::startElement(const QString & namespaceURI, const QString & localName,
																	const QString & qName, const QXmlAttributes & atts )
{
  StackValue val = m_parseStack.top();
  if(val.funcStartElement == NULL) {
    emit recievedInfo(m_name, "Ignoring tag " + localName);
    pushIgnore();
    return true;
  }
  return (this->*val.funcStartElement)(namespaceURI, localName, qName, atts);
}

/** Called when the xml parser encounters a closing tag */
bool KRender::endElement ( const QString & namespaceURI, const QString & localName, const QString & qName )
{
  StackValue val = m_parseStack.pop();
  if(val.funcEndElement == NULL) return true;
  return (this->*val.funcEndElement)(namespaceURI, localName, qName);
}

bool KRender::characters( const QString &ch )
{
  m_characterBuffer += ch;
}


/** Called when the xml parser encounters an opening element and we are outside of a parsing loop. */
bool KRender::topLevelStartElement(const QString & namespaceURI, const QString & localName,
																		 const QString & qName, const QXmlAttributes & atts)
{
	if(localName == "reply") {
		QString command = atts.value("command");
		if(command.isNull()) {
      emit recievedInfo(m_name, "Reply recieved, no command specified");
			return false;
		} else if(command == "createVideoXWindow") {
      StackValue val;
      val.element = "createVideoXWindow";
      val.funcStartElement = &KRender::reply_createVideoXWindow_StartElement;
      val.funcEndElement = 0;
      m_parseStack.push(val);
			return true;
		} else if(command == "getFileProperties") {
			QMap<QString, QString> map;

			map["filename"] = atts.value("filename");
			map["duration"] = atts.value("duration");

			emit replyGetFileProperties(map);

      pushIgnore();
			return true;
		} else if(command == "play") {
      pushIgnore();
      return true;
    } else if(command == "seek") {
      pushIgnore();
      if(m_seekPending) {
        if(m_nextSeek < 0.0) {
          m_seekPending = false;
        } else {
          sendSeekCommand(m_nextSeek);
          m_nextSeek = -1.0;
        }
      }
			return true;
		} else if(command == "render") {
      pushIgnore();
			return true;
    } else if(command == "setSceneList") {
      pushIgnore();
      return true;
    } else {
    	emit recievedInfo(m_name, "Unknown reply '" + command + "'");
      return false;
    }
	} else if(localName == "pong") {
		QString id = atts.value("id");
  	emit recievedInfo(m_name, "pong recieved : " + id);
    pushIgnore();
    return true;
	} else if(localName == "playing") {
    QString tStr = atts.value("time");
    GenTime time(tStr.toDouble());
    pushIgnore();
    emit positionChanged(time);
    return true;
  } else if(localName == "render") {
    QString tStr = atts.value("status");
    pushIgnore();
    if(tStr == "playing") {
      emit playing();
      return true;
    } else if(tStr == "stopped") {
      emit stopped();
      return true;
    }
    emit recievedInfo(m_name, "Render command returned unknown status : '" + tStr + "'");
    return false;
  } else if(localName == "capabilities") {
      StackValue val;
      val.element = "capabilities";
      val.funcStartElement = &KRender::reply_getCapabilities_StartElement;
      val.funcEndElement = 0;
      m_parseStack.push(val);
      return true;
  }

	emit recievedInfo(m_name, "Unknown tag '" + localName + "'");


	return false;
}

bool KRender::reply_getCapabilities_StartElement(const QString & namespaceURI, const QString & localName,
																		 const QString & qName, const QXmlAttributes & atts)
{
  if(localName == "renderer") {
    StackValue val;
    val.element = "renderer";
  	val.funcStartElement = 0;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }
  if(localName == "general") {
    StackValue val;
    val.element = "general";
  	val.funcStartElement = 0;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }
  if(localName == "effects") {
    StackValue val;
    val.element = "effects";
  	val.funcStartElement = &KRender::reply_capabilities_effects_StartElement;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }
  if(localName == "iostreams") {
    StackValue val;
    val.element = "iostreams";
  	val.funcStartElement = &KRender::reply_capabilities_iostreams_StartElement;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }
  if(localName == "codecs") {
    StackValue val;
    val.element = "codecs";
  	val.funcStartElement = &KRender::reply_capabilities_codecs_StartElement;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }

  pushIgnore();
  return true;
}

bool KRender::reply_createVideoXWindow_StartElement(const QString & namespaceURI, const QString & localName,
																		 const QString & qName, const QXmlAttributes & atts)
{
  QString winID = atts.value("WinID");
  WId retID = 0;
  if(winID.isNull()) {
    emit recievedInfo(m_name, "Window ID not specified - emitting 0");
  } else {
    retID = winID.toInt();
  }
	emit replyCreateVideoXWindow(retID);

  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_iostreams_StartElement(const QString & namespaceURI, const QString & localName,
																		 const QString & qName, const QXmlAttributes & atts)
{
  if(localName == "outstream") {
    StackValue val;
    val.element = "outstream";
  	val.funcStartElement = &KRender::reply_capabilities_iostreams_outstream_StartElement;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }

  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_iostreams_outstream_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if(localName == "file") {
    if(m_fileFormat != 0) {
      emit recievedInfo(m_name, "Error - creating new file format, construct pointer is non-null, expect memory leak");
    }
    m_fileFormat = new AVFileFormatDesc("FIXME", "unnamed");
    StackValue val;
    val.element = "file";
  	val.funcStartElement = &KRender::reply_capabilities_iostreams_outstream_file_StartElement;
  	val.funcEndElement = &KRender::reply_capabilities_iostreams_outstream_file_EndElement;
    m_parseStack.push(val);
    return true;
  }

  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_iostreams_outstream_file_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if(localName == "container") {
    if(m_fileFormat == 0) {
      emit recievedInfo(m_name, "Error - inside container, m_fileFormat pointer is null!");
    } else {
      m_fileFormat->setFileExtension(atts.value("extension"));
      QString name = atts.value("format");
      if(!name.isNull()) {
        m_fileFormat->setName(name);
      }
    }
    StackValue val;
    val.element = "container";
  	val.funcStartElement = &KRender::reply_capabilities_iostreams_outstream_container_StartElement;
  	val.funcEndElement = 0;
    m_parseStack.push(val);
    return true;
  }

  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_iostreams_outstream_file_EndElement(const QString & namespaceURI, const QString & localName, const QString & qName)
{
  if(m_fileFormat == 0) {
    emit recievedInfo(m_name, "Error - At end of a file definition, m_fileFormat pointer is null!");
  } else {
    m_fileFormats.append(m_fileFormat);
    emit signalFileFormatsUpdated(m_fileFormats);
    m_fileFormat = 0;
  }
  return true;
}

bool KRender::reply_capabilities_iostreams_outstream_container_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if(localName == "codec") {
    if(m_fileFormat == 0) {
      emit recievedInfo(m_name, "Error - inside codec, m_fileFormat pointer is null!");
    } else {
      if(m_desccodeclist != 0) {
        emit recievedInfo(m_name, "Error - creating codec but m_desccodeclist != 0, expect memory leak");
      }
      QString type = atts.value("type");
      m_desccodeclist = new AVFormatDescCodecList(this, "No description", type);
    }

    StackValue val;
    val.element = "codec";
  	val.funcStartElement = &KRender::reply_capabilities_iostreams_outstream_codec_StartElement;
  	val.funcEndElement = &KRender::reply_capabilities_iostreams_outstream_codec_EndElement;
    m_parseStack.push(val);
    return true;
  }

  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_iostreams_outstream_codec_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if(m_desccodeclist == 0) {
    emit recievedInfo(m_name, "Error - adding codec name, m_desccodeclist pointer is null!");
  } else {
    m_desccodeclist->addCodec(localName);
  }

  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_iostreams_outstream_codec_EndElement(const QString & namespaceURI, const QString & localName, const QString & qName)
{
  if(m_fileFormat == 0) {
    emit recievedInfo(m_name, "Error - At end of a file definition, m_fileFormat pointer is null!");
  } else if(m_desccodeclist == 0) {
    emit recievedInfo(m_name, "Error - At end of a file definition, m_desccodeclist pointer is null!");
  } else {
    m_fileFormat->append(m_desccodeclist);
    m_desccodeclist = 0;
  }

  return true;
}

bool KRender::reply_capabilities_codecs_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if (localName == "encoder") {
    if(m_codec != 0) {
      emit recievedInfo(m_name, "Error - creating codec but one already exists, expect memory leak");
    }
    QString name = atts.value("name");
    m_codec = new AVFormatDescCodec("No description", name);
    emit recievedInfo(m_name, "Creating codec with name " + name);
    
    StackValue val;
    val.element = "encoder";
  	val.funcStartElement = &KRender::reply_capabilities_codecs_encoder_StartElement;
  	val.funcEndElement = &KRender::reply_capabilities_codecs_encoder_EndElement;
    m_parseStack.push(val);
    return true;
  }
  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_codecs_encoder_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if(localName == "about") {
    m_characterBuffer="";
    StackValue val;
    val.element = "about";
  	val.funcStartElement = 0;
  	val.funcEndElement = &KRender::reply_capabilities_codecs_encoder_about_EndElement;
    m_parseStack.push(val);
    return true;
  }
  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_codecs_encoder_EndElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName)
{
  if(m_codec == 0) {
    emit recievedInfo(m_name, "Error - at end of encoder tag, m_codec pointer is null!");
  } else {
    m_codeclist.append(m_codec);
    m_codec = 0;
    emit signalFileFormatsUpdated(m_fileFormats);    
  }
  return true;
}

bool KRender::reply_capabilities_codecs_encoder_about_EndElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName)
{
  if(m_codec == 0) {
    emit recievedInfo(m_name, "Error - at end of about tag, m_codec pointer is null!");
  } else {
    m_codec->setDescription(m_characterBuffer);
  }
  return true;
}

bool KRender::reply_capabilities_effects_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  if(localName == "effect") {
    QString name = atts.value("name");
    if(m_effect != 0) {
      emit recievedInfo(m_name, "Error - creating m_effect but pointer is non-null - expect memory leak");
    }
    m_effect = new EffectDesc(name);    
    
    StackValue val;
    val.element = "effect";
  	val.funcStartElement = &KRender::reply_capabilities_effects_effect_StartElement;
  	val.funcEndElement = &KRender::reply_capabilities_effects_effect_EndElement;
    m_parseStack.push(val);
    return true;
  }
  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_effects_effect_StartElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName,
                                      const QXmlAttributes & atts)
{
  pushIgnore();
  return true;
}

bool KRender::reply_capabilities_effects_effect_EndElement(const QString & namespaceURI,
                                      const QString & localName, const QString & qName)
{
  if(m_effect == 0) {
    emit recievedInfo(m_name, "Error - discovered closing effect tag but m_effect is Null!!!");
  } else {
    m_effectList.append(m_effect);
    m_effect = 0;
  }
  return true;
}