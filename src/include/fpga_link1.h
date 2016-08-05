// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef FPGA_LINK1_H_
#define FPGA_LINK1_H_

#include <pthread.h>
#include <stdint.h>
#include <string>

#include "stopwatch.h"


namespace fpga_link1 {

      
      /**
       * An object of this class lets you send and receive data to an FPGA. At the FPGA side is required a specific
       * hardware to process and respond the commands sent by this software. See vhdl/ directory.
       * 
       * - A serial port UART-8N1 tx and rx blocks. The PC is connected by serial port to the FPGA.
       *
       * - A SLIP Byte Stuffing framer/deframer blocks.
       *
       * - A RMAP (Remote Mameory Access Protocol) block
       *
       * - The system bus can be APB, AHB, Wishbone, etc, this is specified by the digital HW designer and is
       *   transparent to this sofware.
       *
       */
      class FpgaLink1 {
      public:
            
            enum Error {
                  // No error.
                  kErrorNo,
                 
                  // The specified device does not exist.
                  kErrorNoSuchDevice,

                  // Error while creating a the internal thread (with pthread_create).
                  kErrorThreadCreation,
                  
                  // Error during calling tcgetattr(), settcaatr() and other <termios.h> apis
                  kErrorTermios,

                  // One or more parameters are wrong, for example: a data buffer pointer is null or data buffer size is
                  // too large (outside the permited range).
                  kErrorWrongParameters,


                  // Device has informed that the requested read or write operation was unsuccesful (nack)
                  kErrorOperationNotAcknowledged,
                  
                  // Input/output error, for example: during read(), write(), poll(), etc, operations. Comunications.
                  kErrorIO,

                  // Protocol error, device has sent an unexpected frame which does not correspond with the sent one.
                  kErrorProtocol,

                  // Other, not specified, error.
                  kErrorGeneric
            };


            /**
             * Ctor.
             * @param thread_name The name of service thread, this name will show in a GDB session typing "info
             * threads" command.
             */
            FpgaLink1(std::string device);

            /**
             * Dtor.
             */
            ~FpgaLink1();

            /**
             * One-time (heavy) initialization.
             */
            Error Init();


            Error MemoryRD08(int reg, uint8_t* data);
            Error MemoryRD32(int reg, uint32_t* data);
            //Error MemoryRD(int reg, uint8_t* data, int len);

            //
            Error MemoryWR08(int reg, uint8_t data);
            Error MemoryWR32(int reg, uint32_t data);
            
            //Error MemoryWR(int reg, uint8_t* data, int len);

            //
            Error FifoRD(int reg, uint8_t* data, int len);
            Error FifoWR(int reg, uint8_t* data, int len);

      private:

            static const int kIdleSleep = 1000; // 1000 us = 1 ms

            bool initialized_;
            pthread_t thread_;
            pthread_attr_t thread_attr_;
            
            pthread_mutex_t lock_;
            std::string thread_name_;
            bool thread_exit_;

            Stopwatch watch_;
            std::string device_;
            int fd_;
            
            static 
            void* ThreadFn(void* obj);
            void* ThreadFn();

            int RobustWR(int fd, uint8_t* s, int n, int timeout_ms);
            int RobustRD(int fd, uint8_t* s, int n, int timeout_ms);



      };

}


#endif  // FPGALINK1_H_
