#include "logger.h"

Logger::Logger(QObject *parent) : QObject(parent)
{
}

void Logger::openFile(QString fileName, bool trunc)
{
    if (!fileName.isEmpty())
    {
        logFile = new QFile;
        logFile->setFileName(fileName);
        logFile->open(QFile::OpenModeFlag::ReadWrite | QFile::Text);

        if (trunc)
        {
           qDebug() << "trunk";
           logFile->resize(0);
        }
    }
}

void Logger::closeFile()
{
    if (logFile != nullptr)
    {
        logFile->close();
        delete logFile;
        logFile = nullptr;

        qDebug() << "Close File";

        clearWriteBuffer();
    }
}

bool Logger::beginLog(QString path, bool autoLogging, QString fileName, bool truncateFile)
{
    if (logFile == nullptr && QDir(path).isReadable())
    {
        qDebug() << "File open";

        if (autoLogging)
            openFile(path + "/" + QDateTime::currentDateTime().toString("dd.MM.yyyy_hh.mm.ss.zzz_") + "Log" + fileName.right(fileName.length() - fileName.lastIndexOf('.')), truncateFile);
        else
            openFile(path + "/" + fileName, truncateFile);

        if (fileName.contains("csv"))
        {
            csvLabelsBuffer.append(QString(logFile->readLine()).replace("\"", "").split(',', QString::SplitBehavior::SkipEmptyParts));

            for (auto i=0; i < csvLabelsBuffer.count();++i)
                qDebug() << csvLabelsBuffer[i] + "\n";
        }

        return true;
    }
    else
    {
        qDebug() << "Invalid Dir";
        return false;
    }
}

bool Logger::isOpen()
{
    return logFile != nullptr;
}

void Logger::writeLogTXTLine(QString lineToAppend, bool simplifyText)
{
    QString text = lineToAppend; // + "";

    if (simplifyText)
        text = text.simplified();

    text = QDateTime::currentDateTime().toString("hh:mm:ss:zzz ") + text;

    if (logFile != nullptr)
    {
        QTextStream out(logFile);
        out.setCodec("UTF-8");

        out << text + "\r";
    }
}

void Logger::writeLogTXTParsedData(QStringList labelList, QList<double> dataList)
{
    QString text;

    for (auto i = 0; i < labelList.count(); ++i)
    {
        text.append(labelList[i] + " " + QString::number(dataList[i], 'f') + " :: ");
    }

    text = text.simplified();

    text = QDateTime::currentDateTime().toString("hh:mm:ss:zzz ") + text;

    if (logFile != nullptr)
    {
        QTextStream out(logFile);
        out.setCodec("UTF-8");

        out << text + "\r";
    }
}

void Logger::writeLogCSV(QStringList labelList, QList<double> dataList, bool addTime)
{
    QTextStream out(logFile);

    if (addTime)
    {
        if (csvLabelsBuffer.contains("time") == false)
            csvLabelsBuffer.insert(0, "time");
    }

    if (labelList.count() == 0)
    {
        qDebug() << "Empty label list - abort csv write";
        return;
    }

    bool canAddLabel = false;
    for (auto i = 0; i <= labelList.count() - 1; ++i)
    {
        if (csvLabelsBuffer.count() == 0 || !csvLabelsBuffer.contains(labelList[i]))
            canAddLabel = true;

        if (canAddLabel)
        {
            csvLabelsBuffer.append(labelList[i].toLatin1());

            QStringList origFile = out.readAll().split(QRegExp("[\r\n]"), Qt::SplitBehaviorFlags::SkipEmptyParts);
            logFile->resize(0); // delete contents !

            for (auto i = 0; i < csvLabelsBuffer.count(); ++i)
                out << "\"" + csvLabelsBuffer[i] + "\",";

            out << "\n";

            for (auto i = 1; i < origFile.count(); ++i) // Start from second line (data without first line which contains labels)
                out << origFile[i] + "\n\r";

            canAddLabel = false;
           // break;
        }
    }

    if (canAddLabel)
        out << "\r";

    // add Data !
    for (auto i = 0; i < csvLabelsBuffer.count(); ++i)
    {
        out.atEnd(); // ???

        int index = labelList.indexOf(csvLabelsBuffer[i]); // TODO
        if (index >= 0 && index < dataList.count())
            out << QString::number(dataList[index]) + ',';
        else if (csvLabelsBuffer[i] == "time")
            out << QTime::currentTime().toString("hh:mm:ss:zzz") + ',';
    }

    out << "\r";
}

void Logger::clearWriteBuffer()
{
    csvLabelsBuffer.clear();
}
