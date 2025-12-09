# Test vector from RFC 7539 Section 2.3.2
# https://datatracker.ietf.org/doc/html/rfc7539#section-2.1.1
fun Main
    @ key iarray
    @ nonce iarray
    @ counter int

    key : 0 <- 50462976
    key : 1 <- 117835012
    key : 2 <- 185207048
    key : 3 <- 252579084
    key : 4 <- 319951120
    key : 5 <- 387323156
    key : 6 <- 454695192
    key : 7 <- 522067228

    nonce : 0 <- 0
    nonce : 1 <- 1241513984
    nonce : 2 <- 0

    counter = 1

    @ output iarray
    output <- call ChaCha20Block key nonce counter

    @ i int, i = 0
    while i < 16
        @ out int, out <- output : i
        print $ out
        print const \s
        i += 1
    end
    print const \n
ef

fun ChaCha20Block @ key iarray @ nonce iarray @ counter int
    @ state iarray
    
    state : 0 <- 1634760805, state : 1 <- 857760878
    state : 2 <- 2036477234, state : 3 <- 1797285236

    @ tkey int
    tkey <- key : 0, state : 4 <- tkey
    tkey <- key : 1, state : 5 <- tkey
    tkey <- key : 2, state : 6 <- tkey
    tkey <- key : 3, state : 7 <- tkey
    tkey <- key : 4, state : 8 <- tkey
    tkey <- key : 5, state : 9 <- tkey
    tkey <- key : 6, state : 10 <- tkey
    tkey <- key : 7, state : 11 <- tkey

    state : 12 <- counter
    
    @ tnonce int
    tnonce <- nonce : 0, state : 13 <- tnonce
    tnonce <- nonce : 1, state : 14 <- tnonce
    tnonce <- nonce : 2, state : 15 <- tnonce

    @ working_state iarray
    working_state = state

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
    @ i int, i = 0
    while i < 16
        @ wi int, wi <- working_state : i
        @ si int, si <- state : i
        wi += si
        output : i <- wi
        i += 1
    end
    return output
ef

fun Qr & a int & b int & c int & d int
    a = a + b, d <- call Xor d a, d <- call Rotl d 16
    c = c + d, b <- call Xor b c, b <- call Rotl b 12
    a = a + b, d <- call Xor d a, d <- call Rotl d 8
    c = c + d, b <- call Xor b c, b <- call Rotl b 7
ef

fun Rotl @ val int @ n int
    @ part_left int
    @ mult int
    mult <- call NonNegativePower 2 n
    part_left = val * mult

    @ part_right int
    @ param int, param = 32 - n
    part_right <- call Urs val param

    @ temp int
    temp <- call Xor part_left part_right
    return temp
ef

fun Xor @ a int @ b int
    @ signA int, signA = 0
    @ signB int, signB = 0
    @ min_int int
    min_int <- call NonNegativePower 2 31

    if a < 0
        signA = 1
        a -= min_int
    end

    if b < 0
        signB = 1
        b -= min_int
    end

    @ result int, result = 0
    @ placeValue int, placeValue = 1
    @ i int, i = 0
    
    while i < 31
        @ bitA int, bitA = a % 2
        @ bitB int, bitB = b % 2

        @ currentBitXor int
        currentBitXor = bitA + bitB
        
        if currentBitXor = 2
             currentBitXor = 0
        end

        @ multi int
        multi = currentBitXor * placeValue
        result += multi

        placeValue *= 2
        a /= 2
        b /= 2
        i += 1
    end

    @ signSum int, signSum = signA + signB
    if signSum = 1
        result += min_int
    end

    return result
ef

fun Urs @ value int @ bits int
    if bits = 0
       return value
    end

    if bits >= 32
       return 0
    end

    @ divisor int
    divisor <- call NonNegativePower 2 bits

    if value >= 0
        @ ret int
        ret = value / divisor
        return ret
    end

    @ min_int int
    min_int <- call NonNegativePower 2 31 
    
    @ temp int
    temp = value - min_int

    temp /= divisor

    @ shifted_sign int
    @ power int, power = 31 - bits
    shifted_sign <- call NonNegativePower 2 power

    temp += shifted_sign
    
    return temp
ef

fun NonNegativePower @ a int @ b int
    if b < 0
        return
    end

    if b = 0
        return 1
    end

    @ i int, i = 1
    @ res int, res = a
    while i < b
        res *= a
        i += 1
    end
    return res
ef
    


    