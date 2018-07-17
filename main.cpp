#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

#include "bl_msb.h"

SDBlockDevice sd(MBED_CONF_APP_SPI_MOSI, MBED_CONF_APP_SPI_MISO, MBED_CONF_APP_SPI_CLK, MBED_CONF_APP_SPI_CS );
FATFileSystem fs("sd");
FlashIAP flash;

// Write file to flash
void apply_update(FILE *file, uint32_t address);

// Print program start header to console
void printStart(void);

int main()
{
    // Print program start header to console
    printStart();

    if ( 0 != sd.init()) 
    {
        printf(" |" ANSI_COLOR_RED "  %-64s" ANSI_COLOR_RESET "| \r\n", "Error SD-Card init failed, Firmware update will not be loaded" ); 
        
    }
    else
    {
        printf(" |  SD size: %llu %-50s| \r\n", sd.size()/1000000, "MB" ); 

        // Mount FAT
        fs.mount(&sd);
    }

    FILE *file = fopen(UPDATE_FILE, "rb");
    if (file != NULL) {
        printf(" |  %-64s| \r\n", "New update found on SD-Card"); 
        
        apply_update(file, POST_APPLICATION_ADDR);

        fclose(file);
        //remove(UPDATE_FILE);
    } else {

        printf(" |  %-64s| \r\n", "No update found on SD-Card"); 
        if( *((int*)POST_APPLICATION_ADDR) == -1 )
        {
            //No application and no new firmware, load default
            printf(" |  %-64s| \r\n", "Load default"); 
            file = fopen(DEFAULT_FILE, "rb");
            if (file != NULL) {
                printf(" |  %-64s| \r\n", "Default found"); 
                
                apply_update(file, POST_APPLICATION_ADDR);

                fclose(file);
                
            } else {
                printf(" |" ANSI_COLOR_RED "  %-64s" ANSI_COLOR_RESET "| \r\n", "Error no FW loaded");
                printf(" +------------------------------------------------------------------+ \r\n\r\n");  
                return -1;
            }
        }
    }
    
    fs.unmount();
    sd.deinit();

    printf(" |" ANSI_COLOR_GREEN "  %-64s" ANSI_COLOR_RESET "| \r\n", "Starting application"); 
    printf(" +------------------------------------------------------------------+ \r\n\r\n"); 
   
    //Jump to core start
    mbed_start_application(POST_APPLICATION_ADDR);
}

// Write file to flash
void apply_update(FILE *file, uint32_t address)
{
    flash.init();

    const uint32_t page_size = flash.get_page_size();
    char *page_buffer = new char[page_size];
    uint32_t addr = address;
    uint32_t next_sector = addr + flash.get_sector_size(addr);
    bool sector_erased = false;
    while (true) {

        // Read data for this page
        memset(page_buffer, 0, sizeof(page_buffer));
        int size_read = fread(page_buffer, 1, page_size, file);
        if (size_read <= 0) {
            break;
        }

        // Erase this page if it hasn't been erased
        if (!sector_erased) {
            flash.erase(addr, flash.get_sector_size(addr));
            sector_erased = true;
        }

        // Program page
        flash.program(page_buffer, addr, page_size);

        addr += page_size;
        if (addr >= next_sector) {
            next_sector = addr + flash.get_sector_size(addr);
            sector_erased = false;

        }
    }
    delete[] page_buffer;

    flash.deinit();
}

// Print program start header to console
void printStart(void)
{
    printf(CLEAR_CONSOLE);
    printf(FRONT_COLOR_BLACK BACK_COLOR_PEACH );
    printf(" +------------------------------------------------------------------+ \r\n");  
    printf(" |  %-64s| \r\n", "Mbed bootloader version 0.1"); 
    printf(" +------------------------------------------------------------------+ \r\n");    
    printf(" |  Application address: %-46d| \r\n", POST_APPLICATION_ADDR);
    printf(" |  Application size :   %-46d| \r\n", POST_APPLICATION_SIZE);
    printf(" +------------------------------------------------------------------+ \r\n");
    printf(ANSI_COLOR_RESET);

    return; 
}
