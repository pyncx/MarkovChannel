#include "ChannelProtocol.hpp"
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

using namespace std;

using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::Message;


ChannelProtocol::ChannelProtocol(std::string& prototxt)
{
  int fd = open(prototxt.c_str(), O_RDONLY);
  FileInputStream* input = new FileInputStream(fd);
  google::protobuf::TextFormat::Parse(input, &params);

  std::ifstream datfile;
  datfile.open(params.source().c_str());

  if (!datfile) {
    std::cerr << "Unable to Open Source:\t" << params.source() << std::endl;
    exit(-1);
  }

  std::cout << params.source() << std::endl;

  double x; std:string line;
  data = std::vector<double>();
  vars = std::vector<double>();

  while (std::getline(datfile, line)) {
    std::cout << line << std::endl;
    std::istringstream iss(line);
    iss >> x; vars.push_back(x);
    while (iss>>x) {data.push_back(x);}
  }
  delete input;

  n_traces = (int) vars.size();
  v0 = params.v0();
  traces = std::vector<std::vector<Step> >();

  std::cout << n_traces << "\t" << v0 << std::endl;

  for ( int i=0; i<n_traces; i++ ) {
    traces.push_back(std::vector<Step>());
    std::vector<Step>& tmp = traces.back();

    for ( int t=0; t<params.step_size(); t++ ) {

      tmp.push_back(Step()); Step& stp = tmp.back();
      stp.stepsize = params.step(t).stepsize();

      stp.dt = params.step(t).has_dt() ? params.step(t).dt() : vars[i];
      stp.vm = params.step(t).has_vm() ? params.step(t).dt() : vars[i];

      switch(params.step(t).dtype()) {

        case MarkovChannel::ProtocolStep::CONDUCTANCE:
          stp.dtype = CONDUCTANCE; break;

        case MarkovChannel::ProtocolStep::FLUORIMETRY:
          stp.dtype = FLUORIMETRY; break;

        default: break;
      }

      switch(params.step(t).stype()) {

        case MarkovChannel::ProtocolStep::NONE:
          stp.stype = NONE; break;

        case MarkovChannel::ProtocolStep::PEAK:
          stp.stype = PEAK; break;

        case MarkovChannel::ProtocolStep::TAU:
          stp.stype = TAU; break;

        case MarkovChannel::ProtocolStep::TRACE:
          stp.stype = TRACE; break;

        default: break;
      }

      stp.args = std::vector<double>();
      for ( int j=0; j<params.step(t).extra_args_size(); j++ ) {
        stp.args.push_back(params.step(t).extra_args(j));
      }

    }
  }
}


std::ostream& operator<< (std::ostream& os, const Step& step)
{
  os << "dt:\t\t" << step.dt << std::endl;
  os << "vm:\t\t" << step.vm << std::endl;
  os << "stepsize:\t" << step.stepsize << std::endl;

  os << "StepType:\t";
  switch ( step.stype ) {
    case NONE:  os << "NONE " << std::endl; break;
    case PEAK:  os << "PEAK " << std::endl; break;
    case TAU:   os << "TAU  " << std::endl; break;
    case TRACE: os << "TRACE" << std::endl; break;
  }

  return os;
}


std::ostream& operator<< (std::ostream& os, const std::vector<Step>& steps)
{
  for ( int t=0; t<steps.size(); t++ ) {
    os << steps[t] << std::endl;
  }
  return os;

}


// std::ostream& operator<< (std::ostream& os, const ChannelProtocol& proto)
// {
//   os << proto.params.name() << "\n";
//   for (int i = 0; i < proto.data.size(); i++) {
//     os << proto.
//     for (int j = 0; j < proto.data[i].size(); j++) {
//       os << proto.data[i][j] << "\t";
//     }
//     os << "\n";
//   }
//   return os << std::endl;
// }
