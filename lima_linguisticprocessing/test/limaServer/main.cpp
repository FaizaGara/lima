/* This file is part of Klise.
   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>

   Klise is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/


#include "LimaServer.h"
#include <qdir.h>
#include <QByteArray>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <QTimer>

#include "common/LimaCommon.h"
#include "common/QsLog/QsLogCategories.h"
#include "common/XMLConfigurationFiles/xmlConfigurationFileParser.h"
#include "common/XMLConfigurationFiles/xmlConfigurationFileExceptions.h"
// #ifdef WIN32
#include "common/AbstractFactoryPattern/AmosePluginsManager.h"
// #endif
using namespace Lima;
using namespace Lima::Common;
using namespace Lima::Common::XMLConfigurationFiles;

#define DEFAULT_PORT 8080

// to manage supported options.
#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  QsLogging::initQsLog();
  Lima::AmosePluginsManager::single();
  std::cerr << "Amose plugins initialized" << std::endl;
  QsLogging::initQsLog();

  std::string configDir;
  std::string limaServerConfigFile;
  // list of langs to initialize analyzer
  std::vector<std::string> languages;
  // list of pipelines to initialize analyzer
  std::vector<std::string> pipelines;
  int optional_port;
  // time before service stop
  int service_life = 0;
  
  // Declare the supported options.
  po::options_description desc("Usage");
  desc.add_options()
  ("help,h", "Display this help message")
  ("language,l", po::value< std::vector<std::string> >(&languages), "supported languages trigrams")
  ("config-dir", po::value<std::string>(&configDir)->default_value(qgetenv("LIMA_CONF").constData()==0?"":qgetenv("LIMA_CONF").constData(),"$LIMA_CONF"),
                                                                                                                  "Set the directory containing the (LIMA) configuration files")
  ("common-config-file", po::value<std::string>(&limaServerConfigFile)->default_value("lima-server.xml"),
                                                                                  "Set the LIMA server configuration file to use")
  ("pipeline,p", po::value< std::vector<std::string> >(&pipelines), "Set the linguistic analysis supported pipelines")
  ("port", po::value< int >(&optional_port),
   "set the listening port")
  ("service-life,t", po::value< int >(&service_life),
   "set the service life (nb seconds)");
  
  po::positional_options_description p;
  p.add("input-file", -1);
  
  // store value of options
  po::variables_map varMap;
  // parse args and set values in store
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), varMap);
  po::notify(varMap);

  // 
  if (varMap.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  if (configDir.empty())
  {
    configDir = "/usr/share/config/lima/";
  }
  
  // Parse configuration file of lima server
  // options in command line supercede options in configuration file
  // port
  std::string fileName(configDir);
  fileName.append("/").append(limaServerConfigFile);
  XMLConfigurationFileParser configLimaServer(fileName);
  quint16 port = DEFAULT_PORT;
  std::cout << "main: before, port = " << port << std::endl;
  if (varMap.count("port")) {
    port = optional_port;
  }
  else {
    try
    {
      port = QString::fromAscii(configLimaServer.getModuleGroupParamValue("http-server","address","port").c_str()).toInt();
    }
    catch (NoSuchModule& e1)
    {
      qDebug() << "http-server module not defined in config file. Using default port" << DEFAULT_PORT;
    } catch (NoSuchGroup& e2)
    {
      qDebug() << "http-server/address group not defined in config file. Using default port" << DEFAULT_PORT;
    } catch (NoSuchParam& e3)
    {
      qDebug() << "http-server/address/port parameter not defined in config file. Using default port" << DEFAULT_PORT;
    }
  }
  std::cout << "main: after, port = " << port << std::endl;

  // analyzer languages
  {
    std::ostringstream oss;
    std::ostream_iterator<std::string> out_it (oss,", ");
    std::copy ( languages.begin(), languages.end(), out_it );
    std::cout << "main: before, languages = " << oss.str() << std::endl;
  }
  std::deque<std::string> langs;
  if (varMap.count("language")) {
    if( !languages.empty() ) {
      langs.resize(languages.size());
      std::copy(languages.begin(), languages.end(), langs.begin());
    }
  }
  else {
    try
    {
      langs = configLimaServer.getModuleGroupListValues("http-server", "analyzer", "languages") ;
    }
    catch (NoSuchModule& e1)
    {
	qDebug() << "http-server module not defined in config file. Ininialize with all available languages";
    }
    catch (NoSuchParam& e3)
    {
      qDebug() << "http-server/analyzer/languages parameter not defined in config file. Ininialize with all available languages";
    }
  }
  {
    std::ostringstream oss;
    std::ostream_iterator<std::string> out_it (oss,", ");
    std::copy ( langs.begin(), langs.end(), out_it );
    std::cout << "main: after, langs = " << oss.str() << std::endl;
  }

  // analyzer pipelines
  // analyzer languages
  std::deque<std::string> pipes;
  {
    std::ostringstream oss;
    std::ostream_iterator<std::string> out_it (oss,", ");
    std::copy ( pipes.begin(), pipes.end(), out_it );
    std::cout << "main: before, pipes = " << oss.str() << std::endl;
  }
  if (varMap.count("pipelines")) {
    if( !pipelines.empty() ) {
      pipes.resize(pipelines.size());
      std::copy(pipelines.begin(), pipelines.end(), pipes.begin());
    }
  }
  else {
    try
    {
      pipes = configLimaServer.getModuleGroupListValues("http-server", "analyzer", "pipelines") ;
    }
    catch (NoSuchModule& e1)
    {
	qDebug() << "http-server module not defined in config file. Ininialize with all available pipelines";
    }
    catch (NoSuchParam& e3)
    {
      qDebug() << "http-server/analyzer/languages parameter not defined in config file. Ininialize with all available pipelines";
    }
  }
  {
    std::ostringstream oss;
    std::ostream_iterator<std::string> out_it (oss,", ");
    std::copy ( pipes.begin(), pipes.end(), out_it );
    std::cout << "main: after, pipes = " << oss.str() << std::endl;
  }

  QTimer t;
  // Create instance of server
  LimaServer server( configDir, langs, pipes, port, &app, &t);

  if (varMap.count("service-life")) {
    // Stop server and app after service-life seconds
    //note that we need to use t.connect, as main is not a QObject
    t.connect (&t, SIGNAL(timeout()), &server, SLOT(quit()));
    int time_out = service_life*1000;
    t.start(time_out);
  }

  //return app.exec();
  int ret = app.exec();
  std::cout << "main: return " << ret;
  return ret;
}
