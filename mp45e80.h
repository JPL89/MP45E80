// @File		MP45E80.H		 		
// @Author		JOSIMAR PEREIRA LEITE
// @country		Brazil
// @Date		07/04/23
//
//
// Copyright (C) 2023  JOSIMAR PEREIRA LEITE
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
#ifndef MP45E80_H
#define MP45E80_H

#define MP45E80_SCK PORTCbits.RC7
#define MP45E80_SDI PORTCbits.RC6
#define MP45E80_SDO PORTCbits.RC5
#define MP45E80_CS PORTCbits.RC4

#define MP45E80_SCK_TRIS TRISCbits.RC7
#define MP45E80_SDI_TRIS TRISCbits.RC6
#define MP45E80_SDO_TRIS TRISCbits.RC5
#define MP45E80_CS_TRIS TRISCbits.RC4

#define MP45E80_COMMAND_WRITE_ENABLE 0x06
#define MP45E80_COMMAND_WRITE_DISABLE 0x04
#define MP45E80_COMMAND_READ_INDENTIFICATION 0x9F 
#define MP45E80_COMMAND_READ_STATUS 0x05
#define MP45E80_COMMAND_READ_DATA 0x03
#define MP45E80_COMMAND_PAGE_PROGRAM 0x02
#define MP45E80_COMMAND_PAGE_ERASE 0xDB
#define MP45E80_COMMAND_SETOR_ERASE 0xD8
#define MP45E80_COMMAND_DEEP_POWER_DOWN 0xB9
#define MP45E80_COMMAND_RELEASE_FROM_DEEP_POWER_DOWN 0xAB


//----------------------------------------------
//
//
//
void SPI_write(unsigned char data)
{
    for(unsigned char mask = 0x80; mask; mask >>= 1)
    {            
        if(data & mask) MP45E80_SDI = 1; else MP45E80_SDI = 0;  
        
        __asm__ __volatile__("nop"); 
        MP45E80_SCK = 1;
        __asm__ __volatile__("nop");        
        MP45E80_SCK = 0;    
        __asm__ __volatile__("nop");
    }    
}
//----------------------------------------------
//
//
//
unsigned char SPI_read(void)
{
    unsigned char data = 0;
    
    for(unsigned char mask = 0x80; mask; mask >>=1)
    {
        data <<= 1;
        if(MP45E80_SDO) data |= 1;
        
        __asm__ __volatile__("nop");
        MP45E80_SCK = 1;
        __asm__ __volatile__("nop");
        MP45E80_SCK = 0;        
        __asm__ __volatile__("nop");
    }
    
    return ((unsigned char) data);
}
//----------------------------------------------
//
//
//
void MP45E80_WREN(void)
{
    MP45E80_CS = 0;    
    SPI_write(MP45E80_COMMAND_WRITE_ENABLE);
    MP45E80_CS = 1;
}
//----------------------------------------------
//
//
//
void MP45E80_WRDI(void)
{
    MP45E80_CS = 0;    
    SPI_write(MP45E80_COMMAND_WRITE_DISABLE);
    MP45E80_CS = 1;
}
//----------------------------------------------
//
//
//
unsigned char MP45E80_RDSR(void)
{                     
    unsigned char data = 0;
    
    MP45E80_CS = 0;    
    SPI_write(MP45E80_COMMAND_READ_STATUS);
    data = SPI_read();
    MP45E80_CS = 1;        
    return ((unsigned char) data);
}
//----------------------------------------------
//
//
//
void MP45E80_PAGE_ERASE(void)
{
    MP45E80_WREN();
    
    MP45E80_CS = 0;
    SPI_write(MP45E80_COMMAND_PAGE_ERASE); 
    MP45E80_CS = 1;
    
    MP45E80_WRDI();
    
    // CHECK BUSY
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
}
//----------------------------------------------
//
//
//
void MP45E80_SECTOR_ERASE(unsigned long address)
{
    MP45E80_WREN();
    
    MP45E80_CS = 0;
    SPI_write(MP45E80_COMMAND_SETOR_ERASE); 
    SPI_write((unsigned char)((address >> 16) & 0xFF));
    SPI_write((unsigned char)((address >> 8) & 0xFF));
    SPI_write((unsigned char)(address & 0xFF));
    MP45E80_CS = 1;
    
    MP45E80_WRDI();
    
    // CHECK BUSY
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
}
//----------------------------------------------
//
//
//
void MP45E80_WRSR(unsigned char data)
{
    MP45E80_WREN();
    
    MP45E80_CS = 0;
    SPI_write(0x01);
    SPI_write((unsigned char)data);
    MP45E80_CS = 1;
    
    MP45E80_WRDI();
    
    // CHECK BUSY
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
}
//----------------------------------------------
//
//
//
// DEVICE ID 2 BYTE; MANUFACTURER: C2H, MEMORY TYPE: 20H
// DEVICE INDIVIDUAL  13H for IDMP45E80
void MP45E80_RDID(unsigned char *manufacturer,
unsigned char *device_code, unsigned char *uid)
{                 
    MP45E80_CS = 0;    
    SPI_write(MP45E80_COMMAND_READ_INDENTIFICATION);
    *manufacturer = SPI_read();
    *device_code = SPI_read();
    *uid = SPI_read();
    MP45E80_CS = 1;
}
//----------------------------------------------
//
//
//
unsigned char MP45E80_READ_BYTE(unsigned long address)
{         
    unsigned char byte = 0;
    
    MP45E80_CS = 0;    
    SPI_write(MP45E80_COMMAND_READ_DATA);
    SPI_write((unsigned char)((address >> 16) & 0xFF));
    SPI_write((unsigned char)((address >> 8) & 0xFF));
    SPI_write((unsigned char)(address & 0xFF));
    byte = SPI_read();
    MP45E80_CS = 1;
    
    // CHECK BUSY
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
    
    return ((unsigned char) byte);        
}
//----------------------------------------------
//
//
//
void MP45E80_READ_BUFFER(unsigned long address, int size, unsigned char buf[])
{                     
    MP45E80_CS = 0;    
    SPI_write(MP45E80_COMMAND_READ_DATA);    
    
    SPI_write((unsigned char)((address >> 16) & 0xFF));
	SPI_write((unsigned char)((address >> 8) & 0xFF));
	SPI_write((unsigned char)(address & 0xFF));
	
	for(int i = 0; i < size; i++)
    {        
    
        buf[i] = SPI_read();
    }    
    MP45E80_CS = 1;
    
    // CHECK BUSY
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
}
//----------------------------------------------
//
//
//
void MP45E80_PROGRAM_BYTE(unsigned long address, unsigned char data)
{
    MP45E80_WREN();
    
    
    MP45E80_CS = 0;        
    SPI_write(MP45E80_COMMAND_PAGE_PROGRAM);
    SPI_write((unsigned char)((address >> 16) & 0xFF));
    SPI_write((unsigned char)((address >> 8) & 0xFF));
    SPI_write((unsigned char)(address & 0xFF));   
    SPI_write(data);
    MP45E80_CS = 1; 
        
    MP45E80_WRDI();
    
    // CHECK BUSY
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
}
//----------------------------------------------
//
//
//
void MP45E80_PROGRAM_BUFFER(unsigned long address, int size, unsigned char buf[])
{
    MP45E80_WREN();
        
    MP45E80_CS = 0;     
    SPI_write(MP45E80_COMMAND_PAGE_PROGRAM);    
    
    SPI_write((unsigned char)((address >> 16) & 0xFF));
	SPI_write((unsigned char)((address >> 8) & 0xFF));
	SPI_write((unsigned char)(address & 0xFF));
	
	for(int i = 0; i < size; i++)
    {        
        
        SPI_write(buf[i]);
    }    
    MP45E80_CS = 1; 
    
    MP45E80_WRDI();
    
    // CHECK BUSY   
    unsigned char status = 1;
    while((status & 0x01) == 1) status = MP45E80_RDSR();
}
//----------------------------------------------
//
//
//
void MP45E80_Init(void)
{
    MP45E80_SCK_TRIS = 0;
    MP45E80_SDI_TRIS = 0;
    MP45E80_SDO_TRIS = 1;
    MP45E80_CS_TRIS = 0;
    
    MP45E80_CS = 1;
}

#endif 
