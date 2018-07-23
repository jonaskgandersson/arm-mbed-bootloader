/**
 * @brief Class for managing flash application
 * 
 * @file mbedApp.h
 * @author Jonas Andersson
 * @date 2018-07-23
 */

#include "mbed.h"

/**
 * @brief Flash application represantation
 * 
 */
class MbedApp
{
private:
  uint32_t startAddr = NULL;
  uint32_t size = 0;

  char *publicKey = NULL;
  char *checksum = NULL;
  char *sign = NULL;

  uint32_t version = 0;

public:
  /**
     * @brief Write new firmware to flash
     * 
     * @param file Firmeware .bin file to write 
     * @return int Write result, value < 0 if error 
     */
  int update(File file);

  /**
   * @brief Read firmware from flash to file
   * 
   * @param file file to write firmware to 
   * @return int Read result, value < 0 if error
   */
  int read(File file);

  /**
   * @brief Calculate SHA256 of flash application area and compare to
   * application checksum
   * 
   * @return int Verify result, 0 if equal and < 0 if error 
   */
  int verifyChecksum();

  /**
   * @brief Verify application signature to with public key
   * 
   * @return int Verify result, 0 if equal and < 0 if error 
   */
  int verifySignature();
};