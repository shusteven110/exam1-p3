#include <cmath>
#include <iomanip>

#include "SobelFilter.h"

SobelFilter::SobelFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &SobelFilter::blocking_transport);
}

// sobel mask
const int mask[MASK_N][MASK_X][MASK_Y] = {{{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}},

                                          {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}};

void SobelFilter::do_filter() {
  while (true) {
    // for (unsigned int i = 0; i < MASK_N; ++i) {
    //   val[i] = 0;
    // }
    // for (unsigned int v = 0; v < MASK_Y; ++v) {
    //   for (unsigned int u = 0; u < MASK_X; ++u) {
    //     unsigned char grey = (i_r.read() + i_g.read() + i_b.read()) / 3;
    //     for (unsigned int i = 0; i != MASK_N; ++i) {
    //       val[i] += grey * mask[i][u][v];
    //     }
    //   }
    // }
    float temp1 = i_r.read();
    float temp2 = i_r.read();
    float temp3 = i_r.read();
    float result = (temp1/6) + (temp2/3) + (temp3/2);
    // double total = 0;
    // for (unsigned int i = 0; i != MASK_N; ++i) {
    //   total += val[i] * val[i];
    // }
    // int result = (int)(std::sqrt(total));
    o_result.write(result);
    // printf("%f\n",result);
    wait(3); //emulate module delay
  }
}

void SobelFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr = addr - base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case SOBEL_FILTER_RESULT_ADDR:
      buffer.uint = o_result.read();
      break;
    case SOBEL_FILTER_CHECK_ADDR:
      buffer.uint = o_result.num_available();
      break;
    default:
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    break;

  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case SOBEL_FILTER_R_ADDR:
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
      }
      // if (mask_ptr[1] == 0xff) {
      //   i_g.write(data_ptr[1]);
      // }
      // if (mask_ptr[2] == 0xff) {
      //   i_b.write(data_ptr[2]);
      // }
      break;
    default:
      std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
      break;
    }
    break;

  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
