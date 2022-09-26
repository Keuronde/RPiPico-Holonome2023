#include <stdio.h>
#include "spi_nb.h"
#include "hardware/gpio.h"
#include "hardware/structs/spi.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "hardware/regs/dreq.h"





uint16_t spi0_slave_register;
uint8_t* spi0_buffer;
uint8_t  spi0_nb_data_to_read;

#define PIN_CS 1

void cs_select(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

void  cs_deselect(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}



int spi_nb_read_register_8bits(spi_inst_t * spi, uint16_t spi_slave_register, uint8_t *buffer, uint8_t nb_data_to_read){
    uint16_t dummy_buffer[8]={0, 0, 0, 0,0, 0, 0, 0};
    uint8_t nb_data_read;
    static enum {
        INIT,
        WAIT_SPI_IDLE,
        SEND_REGISTER_ADRESS,
        WAIT_SENDING_DATA,
        SEND_DUMMY_DATA,
        WAIT_RECIEVING_DATA,
        READ_DATA,
        
        SPI_IN_ERROR,
    }status=INIT;

    switch(status){
        case INIT:
            if(spi == spi0){
                spi0_slave_register = spi_slave_register;
                spi0_buffer = buffer;
                spi0_nb_data_to_read = nb_data_to_read;
            }
            cs_select();
        case WAIT_SPI_IDLE:
            if(spi_nb_busy(spi) == SPI_IDLE){
                status = SEND_REGISTER_ADRESS;
                //puts("SEND_REGISTER_ADRESS");
            }
            break;
        
        case SEND_REGISTER_ADRESS:
            spi_slave_register = spi_slave_register | 0x80 | 0X40;
            if(spi_nb_write_data(spi, &spi_slave_register, 1) == SPI_OK){
                status = WAIT_SENDING_DATA;
                // Ici on veut tester une interruption
                // Armement de l'interruption
                //puts("WAIT_SENDING_DATA");
            }else{
                status = SPI_IN_ERROR;
            }
            break;
        
        case WAIT_SENDING_DATA:
        
            if(!spi_nb_busy(spi)){
                spi_nb_flush_recieve_fifo(spi);
                status = SEND_DUMMY_DATA;
                //puts("SEND_DUMMY_DATA");
            }
            break;

        case SEND_DUMMY_DATA:
            if(spi_nb_write_data(spi, dummy_buffer, nb_data_to_read) == SPI_OK){
                status = WAIT_RECIEVING_DATA;
                //puts("WAIT_RECIEVING_DATA");
            }else{
                status = SPI_IN_ERROR;
            }
            break;

        case WAIT_RECIEVING_DATA:
            if(!spi_nb_busy(spi)){
                status = READ_DATA;
                //puts("READ_DATA");
            }
            break;

        case READ_DATA:
            cs_deselect();
            nb_data_read = spi_nb_read_data_8bits(spi, buffer);
            if(nb_data_read == nb_data_to_read){
                //puts("SPI_SUCCESS");
                status = INIT;
                return SPI_SUCCESS;
            }
            //puts("SPI_FAILED");
            status = SPI_IN_ERROR;
            return SPI_FAILED;
            break;
        
        case SPI_IN_ERROR:
            //puts("SPI_IN_ERROR");
            spi_nb_flush_recieve_fifo(spi);
            cs_deselect();
            status = INIT;
            return SPI_FAILED;
            break;
    }
    return SPI_IN_PROGRESS;

}

/// @brief Tell if the SPI is busy
/// @param spi SPI device to use (spi0 or spi1)
/// @return SPI_BUSY of SPI_IDLE
int spi_nb_busy(spi_inst_t * spi){
    return (spi_get_hw(spi)->sr & SPI_SSPSR_BSY_BITS);
}


/// @brief Empty SPI Recieve FIFO
/// @param spi SPI device to use (spi0 or spi1)
void spi_nb_flush_recieve_fifo(spi_inst_t * spi){
    uint16_t dummy;
    while(spi_get_hw(spi)->sr & SPI_SSPSR_RNE_BITS){
        dummy = spi_get_hw(spi)->dr;
    }
}

/// @brief read the SPI Recieve FIFO
/// @param spi SPI device to use (spi0 or spi1)
/// @param buffer To store data recieved
/// @return Number of byte read
uint8_t spi_nb_read_data_8bits(spi_inst_t * spi, uint8_t * buffer){
    uint8_t index = 0;
    while(spi_get_hw(spi)->sr & SPI_SSPSR_RNE_BITS){
        buffer[index] = (uint8_t)spi_get_hw(spi)->dr ;//& SPI_SSPDR_DATA_BITS;
        index++;
    }
    return index;
}

/// @brief Write severals byte to the SPI Transmit FIFO
/// @param spi SPI device to use (spi0 or spi1)
/// @param buffer data to transmit
/// @param size size of the data to transmit
/// @return SPI_OK or SPI_ERR_TRANSMIT_FIFO_FULL
inline int spi_nb_write_data(spi_inst_t * spi, uint16_t * buffer, uint8_t size){
    int status_spi = SPI_OK;
    uint8_t index=0;
    do
    {
        if(spi_get_hw(spi)->sr & SPI_SSPSR_TNF_BITS){
            spi_get_hw(spi)->dr = buffer[index];
            status_spi = SPI_OK;
        }else{
            status_spi = SPI_ERR_TRANSMIT_FIFO_FULL;
        }
        while (spi_nb_busy(spi));
        //statu_spi = spi_nb_write_byte(spi, buffer[index]);
        //printf("envoi : %x\n", buffer[index]);
        //sleep_ms(1);
        index++;
    } while ( (status_spi == SPI_OK) && (index < size));
    return status_spi;
}

/// @brief Write one "byte", 4 to 16 bits to the SPI Transmit FIFO.
/// @param spi 
/// @param data : Data to send
/// @return SPI_OK if Ok, SPI_ERR_TRANSMIT_FIFO_FULL if fifo is full
int spi_nb_write_byte(spi_inst_t * spi, uint16_t data){
    if(spi_get_hw(spi)->sr & SPI_SSPSR_TNF_BITS){
        spi_get_hw(spi)->dr = data;
        return SPI_OK;
    }
    return SPI_ERR_TRANSMIT_FIFO_FULL;
}

int spi_read_register(spi_inst_t * spi, uint16_t spi_slave_register, uint8_t *buffer, uint8_t nb_to_read){
    int statu;
    uint8_t nb_read;
    uint16_t tampon[15]={0,0,0,0,0,0,0,0,0,0,0,0};

    spi_slave_register = spi_slave_register | 0x80 | 0X40;
    tampon[0]= spi_slave_register;
    spi_nb_flush_recieve_fifo(spi0);
    cs_select();
    statu = spi_nb_write_data(spi, tampon, 1 + nb_to_read);
    if(statu == SPI_ERR_TRANSMIT_FIFO_FULL){
        printf("Erreur: spi_read_register: SPI_ERR_TRANSMIT_FIFO_FULL");
        return statu;
    }
    while(spi_nb_busy(spi0));
    cs_deselect();
    nb_read = spi_nb_read_data_8bits(spi0, buffer);
    if(nb_read != nb_to_read+1){
        printf("Erreur: spi_read_register, nb de valeurs lues incoherentes");
    }

}