-- ================================================================================ --
-- NEORV32 - Test Setup Using The UART-Bootloader To Upload And Run Executables     --
-- -------------------------------------------------------------------------------- --
-- The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              --
-- Copyright (c) NEORV32 contributors.                                              --
-- Copyright (c) 2020 - 2025 Stephan Nolting. All rights reserved.                  --
-- Licensed under the BSD-3-Clause license, see LICENSE for details.                --
-- SPDX-License-Identifier: BSD-3-Clause                                            --
-- ================================================================================ --

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--library neorv32;
use work.neorv32_package.all;

entity neorv32_test_setup_bootloader is
  generic (
    -- adapt these for your setup --
    CLOCK_FREQUENCY : natural := 27000000; -- clock frequency of clk_i in Hz
    IMEM_SIZE       : natural := 32*1024;   -- size of processor-internal instruction memory in bytes
    DMEM_SIZE       : natural := 16*1024     -- size of processor-internal data memory in bytes
  );
  port (
    -- Global control --
    clk_i       : in  std_ulogic; -- global clock, rising edge
    rstn_i      : in  std_ulogic; -- global reset, low-active, async
    -- GPIO --
    gpio_o      : out std_ulogic_vector(7 downto 0); -- parallel output
    gpio_i      : in  std_ulogic_vector(7 downto 0); -- Thêm dòng này để đọc Button

    -- UART0 --
    uart0_txd_o : out std_ulogic; -- UART0 send data
    uart0_rxd_i : in  std_ulogic;  -- UART0 receive data
    
    -- UART1 (Nối với cảm biến PZEM004T) --- VỊ TRÍ 1: THÊM PORT ---
    uart1_txd_o : out std_ulogic;
    uart1_rxd_i : in  std_ulogic;
    
    -- TWI (I2C) --- THÊM VỊ TRÍ 1 ---
    twi_sda_io  : inout std_logic;
    twi_scl_io  : inout std_logic 
    
  );
end entity;

architecture neorv32_test_setup_bootloader_rtl of neorv32_test_setup_bootloader is

  -- Khai báo tín hiệu trung gian cho TWI
  signal twi_sda_out_internal : std_ulogic;
  signal twi_scl_out_internal : std_ulogic;

  signal con_gpio_out : std_ulogic_vector(31 downto 0);
  signal con_gpio_in  : std_ulogic_vector(31 downto 0) := (others => '0');
  
begin

  -- The Core Of The Problem ----------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  neorv32_top_inst: neorv32_top
  generic map (
    -- Clocking --
    CLOCK_FREQUENCY  => CLOCK_FREQUENCY,   -- clock frequency of clk_i in Hz
    -- Boot Configuration --
    BOOT_MODE_SELECT => 0,                 -- boot via internal bootloader
    -- RISC-V CPU Extensions --
    RISCV_ISA_C      => true,              -- implement compressed extension?
    RISCV_ISA_M      => true,              -- implement mul/div extension?
    RISCV_ISA_Zicntr => true,              -- implement base counters?
    -- Internal Instruction memory --
    IMEM_EN          => true,              -- implement processor-internal instruction memory
    IMEM_SIZE        => IMEM_SIZE, -- size of processor-internal instruction memory in bytes
    -- Internal Data memory --
    DMEM_EN          => true,              -- implement processor-internal data memory
    DMEM_SIZE        => DMEM_SIZE, -- size of processor-internal data memory in bytes
    -- Processor peripherals --
    IO_GPIO_NUM      => 8,                 -- number of GPIO input/output pairs (0..32)
    IO_CLINT_EN      => true,              -- implement core local interruptor (CLINT)?
    IO_UART0_EN      => true,            -- implement primary universal asynchronous receiver/transmitter (UART0)?
    
    -- --- VỊ TRÍ 2: KÍCH HOẠT UART1 ---
    IO_UART1_EN      => true,
    IO_GPTMR_NUM => 1,
    
    -- --- THÊM VỊ TRÍ 2: KÍCH HOẠT TWI ---
    IO_TWI_EN   => true

  )
  port map (
    -- Global control --
    clk_i       => clk_i,        -- global clock, rising edge
    rstn_i      => rstn_i,       -- global reset, low-active, async
    -- GPIO (available if IO_GPIO_NUM > 0) --
    gpio_o      => con_gpio_out, -- parallel output
    gpio_i      => con_gpio_in, -- Nối vào đây

    -- primary UART0 (available if IO_UART0_EN = true) --
    uart0_txd_o => uart0_txd_o,  -- UART0 send data
    uart0_rxd_i => uart0_rxd_i,   -- UART0 receive data

    -- --- VỊ TRÍ 3: NỐI DÂY UART1 ---
    uart1_txd_o => uart1_txd_o,
    uart1_rxd_i => uart1_rxd_i,
    uart1_rtsn_o => open, -- Chỉ cần để open
    uart1_ctsn_i => '1',   -- Phải nối '1' (vì low-active, '1' nghĩa là "cho phép truyền")

    -- --- NỐI DÂY TWI ---
    twi_sda_i  => twi_sda_io, -- Nối thẳng I/O vào chân Input
    twi_sda_o  => twi_sda_out_internal, -- Tín hiệu nội bộ để điều khiển
    twi_scl_i  => twi_scl_io,
    twi_scl_o  => twi_scl_out_internal
    
  );

  -- GPIO output --
  gpio_o <= con_gpio_out(7 downto 0);
  
  -- Gán 8 bit từ chân vật lý vào tín hiệu trung gian 32-bit
  con_gpio_in(7 downto 0) <= gpio_i; 
  -- Các bit còn lại (từ 8 đến 31) phải gán mặc định là '0' để tránh trạng thái trôi (floating)
  con_gpio_in(31 downto 8) <= (others => '0');

  -- Logic đơn giản: 
  -- Nếu output của NEORV32 là '0', chân sẽ bị kéo xuống thấp.
  -- Nếu output của NEORV32 là '1', chân sẽ ở trạng thái 'Z' (trở kháng cao).
  
  twi_sda_io <= '0' when (twi_sda_out_internal = '0') else 'Z';
  twi_scl_io <= '0' when (twi_scl_out_internal = '0') else 'Z';
end architecture;
