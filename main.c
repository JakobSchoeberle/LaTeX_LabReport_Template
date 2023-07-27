#include "project.h"
#include <stdio.h>

volatile int ACCEL = 0, TEST = 0, ZeroCount = 0,Pressed = 0;
int var, count = 0, done = 0, Flip = 0;
uint16 Reg_addr = 0;
static const uint32 accel = 0x0F; //defining the i2c address
static const uint32 fram = 0x50; //defining the i2c address
int main(void)
{
    CyGlobalIntEnable;
    
    uint8 i2c_wr_buf[2050], i2c_rd_buf[2050];
    uint8 i2c_buf[4];
    char uart_out[81];

    I2C_Start();
    
    UART_Start();
    UART_UartPutString("Reset/Reboot\n\r");
    
    /* Configuring CTRL_REG1 */
    i2c_wr_buf[0] = 0x1B; //register address of CTRL_REG1.
    i2c_wr_buf[1] = 0x00; //Set to 0x00
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* Wakeup Function Output Data Rate */
    i2c_wr_buf[0] = 0x1D; //register address of CTRL_REG2.
    i2c_wr_buf[1] = 0x04; //value of CTRL_REG2. (0000 0110) sets the wakeup output data function rate to 12.5 Hz.
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* Configure the accelerometer to create an active high interrupt, on the physical interrupt pin, that latches until INT_REL is read use INT_CTRL_REG1.*/
    i2c_wr_buf[0] = 0x1E; //address of INT_CTRL_REG1.
    i2c_wr_buf[1] = 0x32; //remember my question for Bri about STPOL.
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* Enable interrupts from all six possible directions (+/- x, +/- y, +/- z) use INT_CTRL_REG2*/
    i2c_wr_buf[0] = 0x1F; //address of INT_CTRL_REG2.
    i2c_wr_buf[1] = 0x3F;
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
       
    /* Set the output data rate so that the LPF roll-off matches your wakeup Output Data Rate */
    i2c_wr_buf[0] = 0x21; //address of DATA_CTRL_REG.
    i2c_wr_buf[1] = 0x01; //Set to 12.5 Hz
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* Set the wakeup timer to 10 counts for the threshold. NOTE: You might need to adjust this later for better operation!!! */
    i2c_wr_buf[0] = 0x29; //address of WAKEUP_TIMER.
    i2c_wr_buf[1] = 0x0A; //Set to 10
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* Set the wakeup threshold to be 3g using Equation 1 on page 30 of the KXTJ2 datasheet */
    i2c_wr_buf[0] = 0x6A; //address of WAKEUP_THRESHOLD
    i2c_wr_buf[1] = 0x30; //sets the WAKEUP_THRESHOLD to 48.0 (0x30)
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* Configuring CTRL_REG1 */
    i2c_wr_buf[0] = 0x1B; //register address of CTRL_REG1.
    i2c_wr_buf[1] = 0xD2; //Set to 0xD2 for reasons
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 2, I2C_I2C_MODE_COMPLETE_XFER);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();
    
    /* The accelerometer is now on */
    CyDelay(151); //delay specified in the datasheet
    
    /* Reset accelerometer */
    i2c_wr_buf[0] = 0x1A;
    I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 1, I2C_I2C_MODE_NO_STOP);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
    I2C_I2CMasterClearStatus();  
    I2C_I2CMasterReadBuf(accel, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START);
    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
    I2C_I2CMasterClearStatus();
    
    ACCEL_ISR_Start();
    TEST_ISR_Start();
    
    Red_Write(1);
    Green_Write(1); 
    Blue_Write(1);
    
    for(;;)
    {
        CySysPmDeepSleep();
        
        if (TEST == 1) {
            
            Red_Write(1);
            Green_Write(1); 
            Blue_Write(1);
            
            i2c_buf[0] = 0x00; //Address MSB
            i2c_buf[1] = 0x00; //Address LSB
            count = 0;
            done = 0;
            Reg_addr = 0;
            while(TEST_Read() == 0){
                CyDelay(10);
                count++;
                if(count >= 500){
                    Red_Write(0);
                    Green_Write(0); 
                    Blue_Write(0);
                    while(TEST_Read() == 0);
                    UART_UartPutString("Clear\n\r");
                    
                    /* ------ Clear FRAM HERE ------ */
                    
                        i2c_wr_buf[0] = 0x00;
                        i2c_wr_buf[1] = 0x00;
 
                        for( int i = 2; i <=2048; i++) {
                            i2c_wr_buf[i] = 0x00;
                        }
                           
                        for(int i = 0; i < 65536; i+=2048) {
                            I2C_I2CMasterWriteBuf(fram, i2c_wr_buf, 2048, I2C_I2C_MODE_NO_STOP);
                            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                            I2C_I2CMasterClearStatus();
                            if(Flip == 1){
                                Red_Write(0);
                                Green_Write(0); 
                                Blue_Write(0);
                                Flip = 0;
                            }
                            else{
                                Red_Write(1);
                                Green_Write(1); 
                                Blue_Write(1);
                                Flip = 1;
                            }
                        }

                        UART_UartPutString("Done\n\r");

                }
                else if(Pressed == 0 && TEST_Read() == 1){
                    UART_UartPutString("Reading FRAM\n\r");
                    while(done == 0){

                        /* ------ Read FRAM HERE ------ */
                        i2c_buf[0] = (Reg_addr >> 8) & 0xFF; //Address MSB
                        i2c_buf[1] = Reg_addr & 0xFF; //Address LSB
                        
                        I2C_I2CMasterWriteBuf(fram, i2c_buf, 2, I2C_I2C_MODE_NO_STOP); //2 is always used
                        while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                        I2C_I2CMasterClearStatus();
                        
                        I2C_I2CMasterReadBuf(fram, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START); //2 for Multi-Byte Read. 1 for Single-Byte Read
                        while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
                        I2C_I2CMasterClearStatus();
                        
                        if(i2c_rd_buf[0] == 0x00){
                            snprintf(uart_out, 80, "Done at %d\r\n", Reg_addr);
                            uart_out[80] = 0;
                            UART_UartPutString(uart_out);

                            done = 1;
                        }
                        else{
                            snprintf(uart_out, 80, "Found something at %d\r\n", Reg_addr);
                            uart_out[80] = 0;
                            UART_UartPutString(uart_out);
                            
                            /* ------ Playback from FRAM HERE ------ */
                            if(i2c_rd_buf[0] == 0x01){
                                UART_UartPutString("z-dir\n\r");
                                Red_Write(1);
                                Green_Write(1); 
                                Blue_Write(0);
                                CyDelay(500);
                                Red_Write(1);
                                Green_Write(1); 
                                Blue_Write(1);
                                CyDelay(500);
                            }
                            if(i2c_rd_buf[0] == 0x02){
                                UART_UartPutString("y-dir\n\r");
                                Red_Write(1);
                                Green_Write(0); 
                                Blue_Write(1);
                                CyDelay(500);
                                Red_Write(1);
                                Green_Write(1); 
                                Blue_Write(1);
                                CyDelay(500);
                            }
                            if(i2c_rd_buf[0] == 0x03){
                                UART_UartPutString("x-dir\n\r");
                                Red_Write(0);
                                Green_Write(1); 
                                Blue_Write(1);
                                CyDelay(500);
                                Red_Write(1);
                                Green_Write(1); 
                                Blue_Write(1);
                                CyDelay(500);
                            }

                            Reg_addr++;
                        }
                    }
                    Pressed = 1;
                }
            }
            Red_Write(1);
            Green_Write(1);
            Blue_Write(1);
            
            /* Reset accelerometer */
            i2c_wr_buf[0] = 0x1A;
            I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 1, I2C_I2C_MODE_NO_STOP);
            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
            I2C_I2CMasterClearStatus();
            I2C_I2CMasterReadBuf(accel, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START);
            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
            I2C_I2CMasterClearStatus();

            TEST = 0;
        }
        
        Pressed = 0;
        CyDelay(100);
        
        if (ACCEL == 1) {
            
            i2c_buf[0] = 0x00; //Address MSB
            i2c_buf[1] = 0x00; //Address LSB
            done = 0;
            /* Turn on Leds */
//            Red_Write(1);
//            Green_Write(0); 
//            Blue_Write(1);
//            CyDelay(1000);
            UART_UartPutString("Drop Dectected\n\r");
            
            /* ------ Read ACCEL HERE ------ */
            i2c_wr_buf[0] = 0x17; //INT_SOURCE2
            I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 1, I2C_I2C_MODE_NO_STOP);
            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
            I2C_I2CMasterClearStatus();
            
            I2C_I2CMasterReadBuf(accel, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START);
            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
            I2C_I2CMasterClearStatus();
            
            i2c_buf[3] = 0x00;
            
            done = 0;
            if ((i2c_rd_buf[0] & 0x03) != 0){       //bits 0 and 1 z direction
                i2c_buf[2] = 0x01;
                
                while(done == 0){
                
                i2c_buf[0] = (Reg_addr >> 8) & 0xFF; //Address MSB
                i2c_buf[1] = Reg_addr & 0xFF; //Address LSB
                
                I2C_I2CMasterWriteBuf(fram, i2c_buf, 2, I2C_I2C_MODE_NO_STOP); //2 is always used
                while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                I2C_I2CMasterClearStatus();
                
                I2C_I2CMasterReadBuf(fram, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START); //2 for Multi-Byte Read. 1 for Single-Byte Read
                while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
                I2C_I2CMasterClearStatus();
                
                if(i2c_rd_buf[0] == 0x00){
                    UART_UartPutString("Writing FRAM\r\n");
                    I2C_I2CMasterWriteBuf(fram, i2c_buf, 4, I2C_I2C_MODE_COMPLETE_XFER); //4 for Multi-Byte Write. 3 is used for Single-Byte Write
                    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                    I2C_I2CMasterClearStatus();

                    done = 1;
                    i2c_buf[0] = 0x00; //Address MSB
                    i2c_buf[1] = 0x00; //Address LSB
                }
                else{
                    Reg_addr++;
                    done = 0;
                }
            }
                
                UART_UartPutString("z-dir\n\r");
            }
            done = 0;
            if ((i2c_rd_buf[0] & 0x0C) != 0){        //bits 2 and 3 y green direction
                i2c_buf[2] = 0x02;
                
                while(done == 0){
                
                i2c_buf[0] = (Reg_addr >> 8) & 0xFF; //Address MSB
                i2c_buf[1] = Reg_addr & 0xFF; //Address LSB
                
                I2C_I2CMasterWriteBuf(fram, i2c_buf, 2, I2C_I2C_MODE_NO_STOP); //2 is always used
                while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                I2C_I2CMasterClearStatus();
                
                I2C_I2CMasterReadBuf(fram, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START); //2 for Multi-Byte Read. 1 for Single-Byte Read
                while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
                I2C_I2CMasterClearStatus();
                
                if(i2c_rd_buf[0] == 0x00){
                    UART_UartPutString("Writing FRAM\r\n");
                    I2C_I2CMasterWriteBuf(fram, i2c_buf, 4, I2C_I2C_MODE_COMPLETE_XFER); //4 for Multi-Byte Write. 3 is used for Single-Byte Write
                    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                    I2C_I2CMasterClearStatus();

                    done = 1;
                    i2c_buf[0] = 0x00; //Address MSB
                    i2c_buf[1] = 0x00; //Address LSB
                }
                else{
                    Reg_addr++;
                    done = 0;
                }
            }
                
                UART_UartPutString("y-dir\n\r");
            } 
            done = 0;
            if ((i2c_rd_buf[0] & 0x30) != 0){        //bits 4 and 5 x direction bred
                i2c_buf[2] = 0x03;
                
                while(done == 0){
                
                i2c_buf[0] = (Reg_addr >> 8) & 0xFF; //Address MSB
                i2c_buf[1] = Reg_addr & 0xFF; //Address LSB
                
                I2C_I2CMasterWriteBuf(fram, i2c_buf, 2, I2C_I2C_MODE_NO_STOP); //2 is always used
                while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                I2C_I2CMasterClearStatus();
                
                I2C_I2CMasterReadBuf(fram, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START); //2 for Multi-Byte Read. 1 for Single-Byte Read
                while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
                I2C_I2CMasterClearStatus();
                
                if(i2c_rd_buf[0] == 0x00){
                    UART_UartPutString("Writing FRAM\r\n");
                    I2C_I2CMasterWriteBuf(fram, i2c_buf, 4, I2C_I2C_MODE_COMPLETE_XFER); //4 for Multi-Byte Write. 3 is used for Single-Byte Write
                    while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
                    I2C_I2CMasterClearStatus();

                    done = 1;
                    i2c_buf[0] = 0x00; //Address MSB
                    i2c_buf[1] = 0x00; //Address LSB
                }
                else{
                    Reg_addr++;
                    done = 0;
                }
            }
                
                UART_UartPutString("x-dir\n\r");
            }      
            
            /* Reset accelerometer */
            i2c_wr_buf[0] = 0x1A;
            I2C_I2CMasterWriteBuf(accel, i2c_wr_buf, 1, I2C_I2C_MODE_NO_STOP);
            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT));
            I2C_I2CMasterClearStatus();
            I2C_I2CMasterReadBuf(accel, i2c_rd_buf, 1, I2C_I2C_MODE_REPEAT_START);
            while(!(I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT));
            I2C_I2CMasterClearStatus();
            
            ACCEL = 0;
            i2c_buf[0] = 0x00; //Address MSB
            i2c_buf[1] = 0x00; //Address LSB
        }
        
    }
}

/* [] END OF FILE */
