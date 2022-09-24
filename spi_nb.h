#include "hardware/spi.h"

#define SPI_BUSY 1
#define SPI_IDLE 0

#define SPI_SUCCESS 0
#define SPI_IN_PROGRESS 1
#define SPI_FAILED 2

#define SPI_ERR_TRANSMIT_FIFO_FULL 1
#define SPI_OK 0

int spi_nb_busy(spi_inst_t * spi);
void spi_nb_flush_recieve_fifo(spi_inst_t * spi);
int spi_nb_write_byte(spi_inst_t * spi, uint16_t data);
int spi_nb_write_data(spi_inst_t * spi, uint16_t * buffer, uint8_t size);
uint8_t spi_nb_read_data_8bits(spi_inst_t * spi, uint8_t * buffer);
int spi_nb_read_register_8bits(spi_inst_t * spi, uint16_t spi_slave_register, uint8_t *buffer, uint8_t nb_data_to_read);
int spi_read_register(spi_inst_t * spi, uint16_t spi_slave_register, uint8_t *buffer, uint8_t nb_to_read);

void cs_select(void);
void cs_deselect(void);