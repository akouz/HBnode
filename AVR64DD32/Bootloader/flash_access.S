/**
 * \file flash_access.S
 *
 * \brief Flash access functions.
 *
 (c) 2019 Microchip Technology Inc. and its subsidiaries.
    Subject to your compliance with these terms, you may use this software and
    any derivatives exclusively with Microchip products. It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#include "utils\assembler.h"

PUBLIC_FUNCTION(pgm_byte_read)  
    movw    r30, r24                    // Update Z (R30,R31 pair) with byte address
    lpm	    r24, Z			// Read prog memory byte adressed by Z (pair R30, R31)  
    ret                                 // Return to caller
END_FUNC(pgm_byte_read)

PUBLIC_FUNCTION(pgm_word_write)
    movw    r30, r24                    // Update Z (R30,R31 pair) with address, LSB bit must be 0 
    movw    r0, r22			// Get data word
    spm					// write word from R0,R1 pair to program memory
    clr     r1
    ret                                 // Return to caller
END_FUNC(pgm_word_write)
    
END_FILE()
