/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the GNU General Public License, Version 3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *          http://www.gnu.org/licenses/gpl-3.0.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ARTEMISOPTIONS_H
#define ARTEMISOPTIONS_H

#include <QUrl>
#include <QSet>
#include <QMap>
#include <QString>

namespace artemis
{

enum FormInputGenerationStrategies {
    Random, ConstantString
};

enum PrioritizerStrategies {
    CONSTANT, RANDOM, COVERAGE, READWRITE, ALL_STRATEGIES
};

enum CoverageReport {
    STDOUT, HTML, NONE
};

enum MajorMode {
    AUTOMATED, MANUAL, CONCOLIC
};

enum PathTraceReport {
    ALL_TRACES, CLICK_TRACES, NO_TRACES, HTML_TRACES
};

enum ConcolicTreeOutput {
    TREE_NONE, TREE_FINAL, TREE_ALL
};

enum HeapReport{
    ALL_CALLS, NAMED_CALLS, NO_CALLS
};

enum ExportEventSequence{
    DONT_EXPORT, EXPORT_SELENIUM, EXPORT_JSON
};

enum SMTSolver {
    KALUZA, Z3STR, CVC4
};

typedef struct OptionsType {

    OptionsType() :
        iterationLimit(4),
        numberSameLength(1),
        recreatePage(false),
        disableStateCheck(true),
        concolicNegateLastConstraint(false),
        formInputGenerationStrategy(Random),
        prioritizerStrategy(CONSTANT),
        outputCoverage(NONE),
        majorMode(AUTOMATED),
        reportPathTrace(NO_TRACES),
        concolicTreeOutput(TREE_FINAL),
        concolicTreeOutputOverview(false),
        concolicUnlimitedDepth(false),
        solver(CVC4),
        exportEventSequence(DONT_EXPORT),
        reportHeap(NO_CALLS),
        heapReportFactor(1)
    {}

    QMap<QString, QString> presetFormfields;
    QMap<QString, QString> presetCookies;

    QSet<QUrl> coverageIgnoreUrls;

    int iterationLimit;
    int numberSameLength;

    bool recreatePage;
    bool disableStateCheck;
    bool concolicNegateLastConstraint;

    QString useProxy;
    QString dumpPageStates;

    FormInputGenerationStrategies formInputGenerationStrategy;
    PrioritizerStrategies prioritizerStrategy;
    CoverageReport outputCoverage;

    MajorMode majorMode;

    PathTraceReport reportPathTrace;

    ConcolicTreeOutput concolicTreeOutput;
    bool concolicTreeOutputOverview;
    QString concolicEntryPoint;
    bool concolicUnlimitedDepth;
    SMTSolver solver;

    ExportEventSequence exportEventSequence;

    HeapReport reportHeap;

    int heapReportFactor;


} Options;

}
#endif // ARTEMISOPTIONS_H
