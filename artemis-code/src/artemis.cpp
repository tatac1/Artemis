/*
  Copyright 2011 Simon Holm Jensen. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice, this list of
        conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright notice, this list
        of conditions and the following disclaimer in the documentation and/or other materials
        provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY SIMON HOLM JENSEN ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those of the
  authors and should not be interpreted as representing official policies, either expressed
  or implied, of Simon Holm Jensen
*/
#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <QDir>
#include <QUrl>
#include <QApplication>

#include "exceptionhandlingqapp.h"
#include "runtime/options.h"
#include "artemisapplication.h"
#include "util/loggingutil.h"

using namespace std;

QUrl parseCmd(int argc, char* argv[], artemis::Options& options)
{

    struct option long_options[] = {
        {"strategy-form-input-generation", required_argument, NULL, 'x'},
        {"coverage-report", required_argument, NULL, 'y'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    char c;
    artemis::Log::addLogLevel(artemis::INFO);

    while ((c = getopt_long(argc, argv, "rp:f:t:c:i:v:", long_options, &option_index)) != -1) {

        switch (c) {

        case 'f': {
            QStringList rawformfield = QString(optarg).split("=");
            Q_ASSERT(rawformfield.size() == 2);
            options.presetFormfields.insert(rawformfield.at(0), rawformfield.at(1));
            break;
        }

        case 'p': {
            QDir ld = QDir(QString(optarg));
            options.dumpPageStates = "k";
            break;
        }

        case 'r': {
            options.recreatePage = true;
            break;
        }

        case 't': {
            options.useProxy = QString(optarg);
            break;
        }

        case 'c': {
            QStringList parts = QString(optarg).split("=");
            options.presetCookies.insert(parts.at(0), parts.at(1));
            break;
        }

        case 'i': {
            options.iterationLimit = QString(optarg).toInt();
            break;
        }

        case 'x': {

            if (string(optarg).compare("javascript-constants") == 0) {
                options.formInputGenerationStrategy = artemis::ConstantString;
            } else if (string(optarg).compare("random") == 0) {
                options.formInputGenerationStrategy = artemis::Random;
            } else {
                cerr << "ERROR: Invalid strategy " << optarg << endl;
                exit(1);
            }

            break;

        }

        case 'y': {
            if (string(optarg).compare("html") == 0) {
                options.outputCoverage = artemis::HTML;
            } else if (string(optarg).compare("stdout") == 0) {
                options.outputCoverage = artemis::STDOUT;
            } else if (string(optarg).compare("none") == 0) {
                options.outputCoverage = artemis::NONE;
            } else {
                cerr << "ERROR: Invalid choice of coverage report " << optarg << endl;
                exit(1);
            }
            break;
        }
        case 'v': {
 if(QString(optarg).indexOf("debug",0,Qt::CaseInsensitive)>=0){
                artemis::Log::addLogLevel(artemis::DEBUG);
            }

            if(QString(optarg).indexOf("warning",0,Qt::CaseInsensitive)>=0){
                artemis::Log::addLogLevel(artemis::WARNING);
            }
            if(QString(optarg).indexOf("error",0,Qt::CaseInsensitive)>=0){
                artemis::Log::addLogLevel(artemis::ERROR);
            }
            if(QString(optarg).indexOf("fatal",0,Qt::CaseInsensitive)>=0){
                artemis::Log::addLogLevel(artemis::FATAL);
            }

            if(QString(optarg).indexOf("all",0,Qt::CaseInsensitive)>=0){
                artemis::Log::addLogLevel(artemis::ALL);
            }

            if(QString(optarg).indexOf("off",0,Qt::CaseInsensitive)>=0){
                artemis::Log::addLogLevel(artemis::OFF);

            }

            break;
        }
    }
    }

    if (optind >= argc) {
        cerr << "ERROR: You must specify a URL" << endl;
        exit(1);
    }

    QStringList rawurl = QString(argv[optind]).split("@");
    QUrl url = rawurl.last();

    if (rawurl.size() > 1) {
        QStringList rawauth = rawurl.first().split(":");
        url.setUserName(rawauth.first());
        url.setPassword(rawauth.last());
    }

    return url;
}

void artemisConsoleMessageHandler(QtMsgType type, const char* msg)
{
    switch (type) {
    case QtDebugMsg:
        artemis::Log::debug(QString(msg).toStdString());
        break;
    case QtWarningMsg:
        artemis::Log::warning("Warning: "+QString(msg).toStdString());
        break;
    case QtCriticalMsg:
        artemis::Log::error("Critical: "+QString(msg).toStdString());
        break;
    case QtFatalMsg:
        artemis::Log::fatal("Fatal: "+QString(msg).toStdString());
        abort();
    }
}

int main(int argc, char* argv[])
{
    qInstallMsgHandler(artemisConsoleMessageHandler);

    ExceptionHandlingQApp app(argc, argv);

    artemis::Options options;
    QUrl url = parseCmd(argc, argv, options);

    artemis::ArtemisApplication artemisApp(0, &app, options, url);
    artemisApp.run(url);

    return app.exec();
}
