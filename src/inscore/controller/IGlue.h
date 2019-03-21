/*

  INScore Project

  Copyright (C) 2009,2010  Grame

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


#ifndef __IGlue__
#define __IGlue__

#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QApplication>

#include "IOSCListener.h"
#include "IController.h"
#include "PeriodicTask.h"
#include "udpinfo.h"
#include "Updater.h"
#include "smartpointer.h"


namespace inscore
{

class IMessage;
class IObject;
class IMessageStack;
typedef class libmapping::SMARTP<IObject>		SIObject;
typedef class libmapping::SMARTP<IMessageStack>	SIMessageStack;

//_______________________________________________________________________
/*!
	\brief a specific thread to listen incoming osc packets
*/
class OscThread : public QThread
{
	public:
		SIOSCListener fListener;	
	
				 OscThread(SIMessageStack& msgs, int udpport)  
							 { fListener = IOSCListener::create (msgs, udpport); }
		virtual ~OscThread() { fListener->stop(); quit(), wait(50); }
		/// \brief starts the osc listener
		void run()			 { fListener->run(); }
};

class OSCStream;
class IAppl;
class GraphicUpdateListener;
typedef class libmapping::SMARTP<IAppl>	SIAppl;

//--------------------------------------------------------------------------
/*!	\brief a glue between osc, a basic model and a controller

	IGlue inherits from QObject for timer capabilities.
*/
class inscore_export IGlue : public MsgListener, public QTimer 
{
	OscThread *		fOscThread;
	SUpdater 		fViewUpdater;
	SUpdater 		fLocalMapUpdater;
	SUpdater 		fSlaveMapUpdater;
	SIAppl			fModel;
	SIController	fController;
	SIMessageStack	fMsgStack;
	SIMessageStack	fDelayStack;
	SIMessageStack	fWebMsgStack;
	SPeriodicTask	fTimeTask;
	GraphicUpdateListener * fViewListener;
	QMutex			fTimeViewMutex;
//	int				fCurrentRate;				// the current application rate
//	unsigned long long	fLastTimeTask;			// the date of the last time task (in uls)
	double			fCurrentRate;				// the current application rate
	double			fLastTimeTask;				// the date of the last time task (in mls)

	udpinfo fUDP;
	
	public :
				 IGlue(int udpport, int outport, int errport);
		virtual ~IGlue();


				/*! \brief start running the glue i.e. inscore services

					\param timerInterval a time interval for the time task, expressed in milliseconds
					\param offscreen a boolean value to run without display and to draw the result in an offscreen only.
							This is an experimental implementation intended to support java.
					\param appl the QApplication that runs the program.
					\return a boolean value indicating the running status at start exit
				*/
				bool start(int timerInterval, bool offscreen, QApplication* appl);

                /*!
                 * \brief clean Stop the glue.
                 */
                virtual	void clean ();

                /*!
                 * \brief restart Restart the glue after a stop with clean method.
                 */
                virtual void restart();

				void setViewUpdater(SUpdater updater);
				void setLocalMapUpdater(SUpdater updater);
				void setSlaveMapUpdater(SUpdater updater);

				void msgReceived (const IMessage* msg, int status);

				void setOSCOut (const std::string& a);
				void setOSCErr (const std::string& a);

				void setGraphicListener(GraphicUpdateListener* listener)	{ fViewListener = listener; }
				bool getSceneView(unsigned int* dest, int w, int h, bool smooth=false );
				const IObject* root () const;

		virtual void timerEvent ( QTimerEvent * event );

		static void trace (const IMessage* msg, int status);

	protected:
		virtual void initialize (bool offscreen,  QApplication* appl);
		
		void modelUpdate();
		void localMapUpdate();
		void slaveMapUpdate();
		void viewUpdate();

		void oscinit (SIAppl appl, udpinfo& udp);								// osc streams initialization
		void oscinit (int port);												// input stream initialization
		void oscinit (OSCStream& osc, const std::string& address, int port);	// outut osc stream initialization
		void checkUDPChange ();
		void setOSCOut (int port);
		void setOSCErr (int port);
};

} // end namespoace

#endif
