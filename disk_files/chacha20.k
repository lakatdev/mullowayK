fun Main
    @ key iarray
    @ i int, i = 0
    while i < 8, key : i <- 0, i += 1, end
    
    @ nonce iarray
    i = 0
    while i < 3, nonce : i <- 0, i += 1, end
    
    @ counter int, counter = 1
    
    @ plaintext string
    cat plaintext const kecske
    
    @ ciphertext string
    ciphertext <- call ChaCha20 key nonce counter plaintext
    
    print const Ciphertext: \n
    print string ciphertext
    print const \n
ef

fun ChaCha20 @ key iarray @ nonce iarray @ counter int @ text string
    @ plaintext_bytes iarray
    plaintext_bytes <- call StringToBytes text
    
    @ ciphertext_bytes iarray
    ciphertext_bytes <- call EncryptBytes plaintext_bytes key nonce counter
    
    @ result string
    result <- call BytesToHex ciphertext_bytes
    return result
ef

fun EncryptBytes @ input iarray @ key iarray @ nonce iarray @ counter int
    @ output iarray
    @ in_len int
    in_len sizeof input
    
    @ byte_idx int, byte_idx = 0
    
    while byte_idx < in_len
        @ keystream iarray
        keystream <- call ChaCha20Block key nonce counter
        counter += 1
        
        @ block_byte_idx int, block_byte_idx = 0
        while block_byte_idx < 64
            if byte_idx < in_len
                @ word_idx int
                word_idx = block_byte_idx / 4
                
                @ byte_in_word int
                @ temp_mod int
                temp_mod = block_byte_idx % 4
                byte_in_word = temp_mod
                
                @ shift int
                shift = byte_in_word * 8
                
                @ keystream_word int
                keystream_word <- keystream : word_idx
                
                @ keystream_byte int
                keystream_byte <- call Urs keystream_word shift
                keystream_byte = keystream_byte % 256
                if keystream_byte < 0
                    keystream_byte += 256
                end
                
                @ input_byte int
                input_byte <- input : byte_idx
                if input_byte < 0
                    input_byte += 256
                end
                
                @ output_byte int
                output_byte <- call ByteXor input_byte keystream_byte
                
                output : byte_idx <- output_byte
                byte_idx += 1
            end
            
            block_byte_idx += 1
            if byte_idx >= in_len
                block_byte_idx = 100
            end
        end
    end
    
    return output
ef

fun StringToBytes @ s string
    @ bytes iarray
    @ len int
    len sizeof s
    
    @ i int, i = 0
    while i < len
        @ char_byte byte
        char_byte <- s : i
        bytes : i <- char_byte
        i += 1
    end
    
    return bytes
ef

fun BytesToHex @ bytes iarray
    @ hex string
    @ len int
    len sizeof bytes
    @ i int, i = 0
    while i < len
        @ val int
        val <- bytes : i
        if val < 0
            val += 256
        end
        @ h1 int
        h1 = val / 16
        @ h2 int
        h2 = val % 16
        @ c1 string
        c1 <- call NibbleToChar h1
        @ c2 string
        c2 <- call NibbleToChar h2
        cat hex string c1
        cat hex string c2
        i += 1
    end
    return hex
ef

fun NibbleToChar @ n int
    @ s string
    if n = 0, cat s const 0, return s, end
    if n = 1, cat s const 1, return s, end
    if n = 2, cat s const 2, return s, end
    if n = 3, cat s const 3, return s, end
    if n = 4, cat s const 4, return s, end
    if n = 5, cat s const 5, return s, end
    if n = 6, cat s const 6, return s, end
    if n = 7, cat s const 7, return s, end
    if n = 8, cat s const 8, return s, end
    if n = 9, cat s const 9, return s, end
    if n = 10, cat s const A, return s, end
    if n = 11, cat s const B, return s, end
    if n = 12, cat s const C, return s, end
    if n = 13, cat s const D, return s, end
    if n = 14, cat s const E, return s, end
    if n = 15, cat s const F, return s, end
    cat s const ?
    return s
ef

fun ChaCha20Block @ key iarray @ nonce iarray @ counter int
    @ state iarray
    state : 0 <- 1634760805, state : 1 <- 857760878
    state : 2 <- 2036477234, state : 3 <- 1797285236

    @ tkey int
    tkey <- key : 0
    tkey <- call Xor tkey 0
    state : 4 <- tkey
    tkey <- key : 1
    tkey <- call Xor tkey 0
    state : 5 <- tkey
    tkey <- key : 2
    tkey <- call Xor tkey 0
    state : 6 <- tkey
    tkey <- key : 3
    tkey <- call Xor tkey 0
    state : 7 <- tkey
    tkey <- key : 4
    tkey <- call Xor tkey 0
    state : 8 <- tkey
    tkey <- key : 5
    tkey <- call Xor tkey 0
    state : 9 <- tkey
    tkey <- key : 6
    tkey <- call Xor tkey 0
    state : 10 <- tkey
    tkey <- key : 7
    tkey <- call Xor tkey 0
    state : 11 <- tkey

    state : 12 <- counter
    @ tnonce int
    tnonce <- nonce : 0
    tnonce <- call Xor tnonce 0
    state : 13 <- tnonce
    tnonce <- nonce : 1
    tnonce <- call Xor tnonce 0
    state : 14 <- tnonce
    tnonce <- nonce : 2
    tnonce <- call Xor tnonce 0
    state : 15 <- tnonce

    @ working_state iarray
    @ i int, i = 0
    while i < 16
        @ s_val int, s_val <- state : i
        working_state : i <- s_val  
        i += 1
    end

    @ loop_count int, loop_count = 0
    while loop_count < 10
        @ a int, @ b int, @ c int, @ d int
        a <- working_state : 0, b <- working_state : 4
        c <- working_state : 8, d <- working_state : 12
        call Qr a b c d
        working_state : 0 <- a, working_state : 4 <- b
        working_state : 8 <- c, working_state : 12 <- d

        a <- working_state : 1, b <- working_state : 5
        c <- working_state : 9, d <- working_state : 13
        call Qr a b c d
        working_state : 1 <- a, working_state : 5 <- b
        working_state : 9 <- c, working_state : 13 <- d

        a <- working_state : 2, b <- working_state : 6
        c <- working_state : 10, d <- working_state : 14
        call Qr a b c d
        working_state : 2 <- a, working_state : 6 <- b
        working_state : 10 <- c, working_state : 14 <- d

        a <- working_state : 3, b <- working_state : 7
        c <- working_state : 11, d <- working_state : 15
        call Qr a b c d
        working_state : 3 <- a, working_state : 7 <- b
        working_state : 11 <- c, working_state : 15 <- d

        a <- working_state : 0, b <- working_state : 5
        c <- working_state : 10, d <- working_state : 15
        call Qr a b c d
        working_state : 0 <- a, working_state : 5 <- b
        working_state : 10 <- c, working_state : 15 <- d

        a <- working_state : 1, b <- working_state : 6
        c <- working_state : 11, d <- working_state : 12
        call Qr a b c d
        working_state : 1 <- a, working_state : 6 <- b
        working_state : 11 <- c, working_state : 12 <- d

        a <- working_state : 2, b <- working_state : 7
        c <- working_state : 8, d <- working_state : 13
        call Qr a b c d
        working_state : 2 <- a, working_state : 7 <- b
        working_state : 8 <- c, working_state : 13 <- d

        a <- working_state : 3, b <- working_state : 4
        c <- working_state : 9, d <- working_state : 14
        call Qr a b c d
        working_state : 3 <- a, working_state : 4 <- b
        working_state : 9 <- c, working_state : 14 <- d

        loop_count += 1
    end

    @ output iarray
    i = 0
    while i < 16
        @ wi int, wi <- working_state : i
        @ si int, si <- state : i
        wi += si
        wi <- call Xor wi 0
        output : i <- wi
        i += 1
    end
    return output
ef

fun Qr & a int & b int & c int & d int
    a = a + b
    a <- call Xor a 0
    d <- call Xor d a
    d <- call Rotl d 16
    
    c = c + d
    c <- call Xor c 0
    b <- call Xor b c
    b <- call Rotl b 12
    
    a = a + b
    a <- call Xor a 0
    d <- call Xor d a
    d <- call Rotl d 8
    
    c = c + d
    c <- call Xor c 0
    b <- call Xor b c
    b <- call Rotl b 7
ef

fun Rotl @ val int @ n int
    val <- call Xor val 0
    
    @ param int, param = 32 - n
    
    @ part_right int
    part_right <- call Urs val param

    @ mask_bits int
    mask_bits <- call NonNegativePower 2 param
    
    @ val_masked int
    val_masked = val % mask_bits
    if val_masked < 0, val_masked += mask_bits, end
    
    @ mult int
    mult <- call NonNegativePower 2 n
    @ part_left int
    part_left = val_masked * mult
    
    @ temp int
    temp <- call Xor part_left part_right
    return temp
ef

fun Urs @ value int @ bits int
    if bits = 0, return value, end
    if bits >= 32, return 0, end
    
    @ divisor int
    divisor <- call NonNegativePower 2 bits
    
    if value >= 0
        @ ret int
        ret = value / divisor
        return ret
    end
    
    @ min_int int
    min_int <- call NonNegativePower 2 31
    
    @ val_low int
    val_low = value + min_int
    val_low = val_low / divisor
    
    @ power_bit int
    @ p_exp int, p_exp = 31 - bits
    power_bit <- call NonNegativePower 2 p_exp
    
    @ result int
    result = val_low + power_bit
    return result
ef

fun ByteXor @ a int @ b int
    @ res int, res = 0
    @ pv int, pv = 1
    @ i int, i = 0
    while i < 8
        @ ba int, ba = a % 2
        @ bb int, bb = b % 2
        @ sum int, sum = ba + bb
        if sum = 2, sum = 0, end
        @ term int, term = sum * pv
        res += term
        pv *= 2
        a /= 2
        b /= 2
        i += 1
    end
    return res
ef

fun Xor @ a int @ b int
    @ signA int, signA = 0
    @ signB int, signB = 0
    @ min_int int, min_int = 2147483648

    if a < 0, signA = 1, a += min_int, end
    if b < 0, signB = 1, b += min_int, end

    @ result int, result = 0
    @ placeValue int, placeValue = 1
    @ i int, i = 0
    
    while i < 31
        @ bitA int, bitA = a % 2
        @ bitB int, bitB = b % 2
        @ currentBitXor int
        currentBitXor = bitA + bitB
        if currentBitXor = 2, currentBitXor = 0, end
        @ multi int
        multi = currentBitXor * placeValue
        result += multi
        placeValue *= 2
        a /= 2
        b /= 2
        i += 1
    end

    @ signSum int, signSum = signA + signB
    if signSum = 1, result -= min_int, end
    return result
ef

fun NonNegativePower @ a int @ b int
    if b < 0, return 0, end
    if b = 0, return 1, end
    @ i int, i = 1
    @ res int, res = a
    while i < b
        res *= a
        i += 1
    end
    return res
ef
