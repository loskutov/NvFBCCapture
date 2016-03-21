#include <NvFBCLibrary.h>
#include <NvFBC/nvFBC.h>
#include <NvFBC/nvFBCH264.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

const NvU32 FPS = 30;

using namespace std;

enum class profiles
{
    BASE = 66,
    MAIN = 77,
    HIGH = 100
};

istream& operator>>(std::istream& in, profiles& profile)
{
	string token;
	in >> token;
	if (token == "BASE")
		profile = profiles::BASE;
	else if (token == "MAIN")
		profile = profiles::MAIN;
	else if (token == "HIGH")
		profile = profiles::HIGH;
	else
		in.setstate(ios_base::failbit);
	return in;
}

ostream& operator<<(std::ostream& out, profiles const& profile)
{
	switch (profile) {
		case profiles::BASE:
			out << "BASE";
			break;
		case profiles::MAIN:
			out << "MAIN";
			break;
		case profiles::HIGH:
			out << "HIGH";
			break;
	}
	return out;
}

// Command line arguments
struct cmdargs
{
    NvU32    frame_cnt;
    NvU32    bitrate;
    profiles profile;
    string   filename;
    bool     is_lossless;
    bool     bYUV444;
};

int main(int argc, char *argv[])
{
	cmdargs args;
	namespace po = boost::program_options;
	po::options_description desc("Usage");
	desc.add_options()
		("frames,f",   po::value<NvU32>(&args.frame_cnt)->required()->default_value(FPS), "Number of frames to capture")
		("bitrate,b",  po::value<NvU32>(&args.bitrate)->default_value(8'000'000), "The desired average bitrate")
		("profile,p",  po::value<profiles>(&args.profile)->default_value(profiles::MAIN), "The encoding profile (BASE/MAIN/HIGH)")
		("output,o",   po::value<string>(&args.filename)->default_value("stream.h264"), "The filename for the output stream")
		("lossless,l", po::bool_switch(&args.is_lossless), "If set, the frames are encoded lossless")
		("yuv444",     po::bool_switch(&args.bYUV444),     "If set, YUV444 encoding is enabled, hence no color resampling performed")
		;

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);
	} catch (boost::program_options::error const& e) {
		cerr << e.what() << endl;
		cout << desc;
		return EXIT_FAILURE;
	}
    
    DWORD max_width, max_height;

    NvFBCLibrary nvfbc;

    NvFBCFrameGrabInfo grab_info = {0};
    NvFBC_H264HWEncoder_FrameInfo frame_info = {0};
    NVFBC_H264_GRAB_FRAME_PARAMS fbch264GrabFrameParams = {0};
    NVFBCRESULT res;
    
	if (!nvfbc.load()) {
		cerr << "Cannot load NvFBC library" << endl;
		return EXIT_FAILURE;
	}

    // Create the encoder instance
	unique_ptr<NvFBCToH264HWEncoder, decltype(bind(&NvFBCToH264HWEncoder::NvFBCH264Release, placeholders::_1))> encoder {
		static_cast<NvFBCToH264HWEncoder*>(nvfbc.create(NVFBC_TO_H264_HW_ENCODER, &max_width, &max_height)),
		bind(&NvFBCToH264HWEncoder::NvFBCH264Release, placeholders::_1) };

    if (!encoder) {
        cerr << "Cannot create the H.264 encoder\n";
        return EXIT_FAILURE;
    }

    ofstream output_file { args.filename, ios::binary };

    if (!output_file) {
        cerr << "Cannot open " << args.filename << " for writing\n";
        return EXIT_FAILURE;
    }

    vector<NvU8> output_buffer(max_width * max_height);

    NvFBC_H264HWEncoder_Config encode_config = {0};
    encode_config.dwVersion = NVFBC_H264HWENC_CONFIG_VER;
    encode_config.dwProfile = static_cast<DWORD>(args.profile);
    encode_config.dwFrameRateNum = FPS;
    encode_config.dwFrameRateDen = 1;  // fps == fps / 1
    encode_config.bOutBandSPSPPS = FALSE; // Use inband SPSPPS, if you need to grab headers on demand use outband SPSPPS
    encode_config.bRecordTimeStamps = TRUE; // Do record timestamps
    encode_config.stereoFormat = NVFBC_H264_STEREO_NONE;
    
    if (args.bYUV444)
        encode_config.bEnableYUV444Encoding = TRUE;

    if (args.is_lossless) {
        encode_config.ePresetConfig = NVFBC_H264_PRESET_LOSSLESS_HP;
        encode_config.eRateControl  = NVFBC_H264_ENC_PARAMS_RC_CONSTQP;
    } else {
        encode_config.dwAvgBitRate = args.bitrate;
        encode_config.dwPeakBitRate = args.bitrate * 2; // Set the peak bitrate twice of the average
        encode_config.dwGOPLength = 100; // The keyframe frequency
        encode_config.eRateControl = NVFBC_H264_ENC_PARAMS_RC_VBR; // Variable bitrate
        encode_config.ePresetConfig= NVFBC_H264_PRESET_LOW_LATENCY_HQ;
        encode_config.dwQP = 26; // Quantization parameter, between 0 and 51 
    }

    NVFBC_H264_SETUP_PARAMS fbch264SetupParams = {0};
    fbch264SetupParams.dwVersion = NVFBC_H264_SETUP_PARAMS_VER;
    fbch264SetupParams.bWithHWCursor = TRUE;
    fbch264SetupParams.pEncodeConfig = &encode_config;

    res = encoder->NvFBCH264SetUp(&fbch264SetupParams);
    if (res != NVFBC_SUCCESS) {
        cerr << "Cannot setup H264 encoder\n";
        return EXIT_FAILURE;
    }

    for (unsigned i = 0; i < args.frame_cnt; ++i) {
        memset(&grab_info, 0, sizeof(grab_info));
        memset(&frame_info, 0, sizeof(frame_info));
        memset(&fbch264GrabFrameParams, 0, sizeof(fbch264GrabFrameParams));
        fbch264GrabFrameParams.dwVersion = NVFBC_H264_GRAB_FRAME_PARAMS_VER;
        fbch264GrabFrameParams.dwFlags   = NVFBC_TOH264_NOWAIT;
        fbch264GrabFrameParams.pNvFBCFrameGrabInfo = &grab_info;
        fbch264GrabFrameParams.pFrameInfo = &frame_info;
        fbch264GrabFrameParams.pBitStreamBuffer = output_buffer.data();

        res = encoder->NvFBCH264GrabFrame(&fbch264GrabFrameParams); // blocks until a new frame available
        if (res == NVFBC_SUCCESS) {
            if (frame_info.dwByteSize == 0) {
                cerr << "Got zero-sized frame\n";
                return EXIT_FAILURE;
            }

            output_file.write(reinterpret_cast<const char*>(output_buffer.data()), frame_info.dwByteSize);

            cerr << "Wrote frame " << i << " to " << args.filename << endl;
        } else {
            if (res == NVFBC_ERROR_INVALIDATED_SESSION) {
				// Invalidated session: need to re-create the encoder...
                encoder.reset(static_cast<NvFBCToH264HWEncoder *>(nvfbc.create(NVFBC_TO_H264_HW_ENCODER, &max_width, &max_height)));
                res = encoder->NvFBCH264SetUp(&fbch264SetupParams);
				// ...and then try again
				if (res == NVFBC_SUCCESS) {
					output_buffer.resize(max_width * max_height);
					res = encoder->NvFBCH264GrabFrame(&fbch264GrabFrameParams);
				}
            }
            if (res != NVFBC_SUCCESS) {
                cerr << "Cannot grab the frame\n";
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}