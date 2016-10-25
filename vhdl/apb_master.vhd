LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

entity apb_master is
   PORT( 
      clk      : in     std_logic;
      reset_n  : in     std_logic;
      -- rx frame
      frame_dec_data  : in  std_logic_vector(61 downto 00);
      frame_dec_valid : in  std_logic;
      frame_dec_ready : out std_logic;  
      -- tx frame
      frame_enc_data  : out std_logic_vector(61 downto 00);
      frame_enc_valid : out std_logic;
      frame_enc_ready : in  std_logic;   
      -- APB Slaves 
      psel         : out    std_logic;
      pwrite       : out    std_logic;
      penable      : out    std_logic;
      paddr        : out    std_logic_vector(23 DOWNTO 00);  
      pwdata       : out    std_logic_vector(31 DOWNTO 00);
      prdata       : in     std_logic_vector(31 DOWNTO 00);
      pready       : in     std_logic
   );

end apb_master ;




architecture rtl of apb_master is
      
  --APB internals 
  signal rx_code        : unsigned(05 downto 00); -- 6 bits
  signal paddr_int      : std_logic_vector(23 downto 00); 
 
  constant CODE_IDLE         : natural := 1;
  constant CODE_READ32       : natural := 2;
  constant CODE_WRITE32      : natural := 3;
  constant CODE_IDLE_ACK     : natural := 8;
  constant CODE_READ32_ACK   : natural := 9;
  constant CODE_READ32_NACK  : natural := 10;
  constant CODE_WRITE32_ACK  : natural := 11;
  constant CODE_WRITE32_NACK : natural := 12;
  constant CODE_INTERRUPT    : natural := 15;
  
begin
  paddr <= paddr_int;
  
  -- rx frame: address (24 bits) and data (32 bits) buses
  rx_code     <= unsigned(frame_dec_data(61 downto 56));
  paddr_int   <= frame_dec_data(55 downto 32);
  pwdata      <= frame_dec_data(31 downto 00);
  
  -- tx frame:
  -- read data from apb slave (32 bits).
  frame_enc_data(61 downto 56) <= B"000100";
  frame_enc_data(55 downto 32) <= paddr_int;
  frame_enc_data(31 downto 00) <= prdata;
  frame_enc_valid <= '0'; --todo

 CTL : block
   type   state_type is (IDLE, RD0, RD1, WR0, WR1, RDY);
   signal state : state_type;
   signal op    : std_logic_vector(6 downto 0);
 begin

   -- Graph transitions
   process (reset_n, clk)
   begin
     if reset_n = '0' then
       state <= IDLE;
		 
     elsif rising_edge(clk) then
       case (state) is
         when IDLE => 
          if frame_dec_valid = '1' then 
            --if rx_code = CODE_READ32 then
              --state <= RD0;
            --elsif rx_code = CODE_WRITE32 then
              state <= WR0; 
            --end if;
          end if;
         when RD0  => 
				state <= RD1;
         when RD1  => 
            if pready = '1' then
              state <= RDY;
            end if;
         when WR0  => 
				state <= WR1;
         when WR1  => 
            if pready = '1' then
              state <= RDY;
            end if;
         when RDY  => 
				state <= IDLE;
       end case;
     end if;
   end process;

----------------------------------------------------------------------------------------------------
   -- control signals for data path
   process (state)
   begin
     case state is
       when IDLE => psel <= '0'; penable <= '0'; pwrite <= '0'; frame_dec_ready <= '0';
       when RD0  => psel <= '1'; penable <= '0'; pwrite <= '0'; frame_dec_ready <= '0';
       when RD1  => psel <= '1'; penable <= '1'; pwrite <= '0'; frame_dec_ready <= '0';
       when WR0  => psel <= '1'; penable <= '0'; pwrite <= '1'; frame_dec_ready <= '0';
       when WR1  => psel <= '1'; penable <= '1'; pwrite <= '1'; frame_dec_ready <= '0';
       when RDY  => psel <= '0'; penable <= '0'; pwrite <= '0'; frame_dec_ready <= '1';
     end case;
   end process;

 end block;



end rtl;