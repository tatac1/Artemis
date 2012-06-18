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
#ifndef ABSTRACTINPUTGENERATOR_H
#define ABSTRACTINPUTGENERATOR_H

#include <QObject>
#include <QApplication>

#include "artemisoptions.h"
#include "executionresult.h"
#include "worklist/worklist.h"
#include "webkitexecutor.h"
#include "executorstate.h"
#include "urls/urlcollector.h"
#include "listeners/artemistopexecutionlistener.h"
#include "coverage/codecoverage.h"

namespace artemis {

    class InputGeneratorStrategy : public QObject
    {
        Q_OBJECT
    public:
        InputGeneratorStrategy(QObject *parent, ArtemisOptions* options);
        ~InputGeneratorStrategy();

        virtual void add_new_configurations(const ExecutableConfiguration*, const ExecutionResult&, WorkList*, ExecutorState* exe_state) = 0;
        virtual void reprioritize() = 0;


    protected:
        ArtemisOptions* artemis_options;
        int iterations;

    };

}
#endif // ABSTRACTINPUTGENERATOR_H