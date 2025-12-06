fun Create & data string & lens iarray
    free data
    free lens
ef

fun Clean & data string & lens iarray
    free data
    free lens
ef

fun Get & data string & lens iarray @ pos int
    @ ret_str string
    @ length int, length sizeof data

    if length <= pos
        return
    end

    length <- lens : int
    @ abs_pos int, abs_pos = 0

    @ i int, i = 0
    while i < pos
        @ curr int
        curr <- lens : i
        abs_pos += curr
        i += 1
    end

    i = 0
    while i < length
        @ curr_pos int, curr_pos = abs_pos + i
        @ char byte
        char <- data : curr_pos
        ret_str : i <- char
        i += 1
    end

    return ret_str
ef

fun Add & data string & lens iarray @ item string
    @ len int, len sizeof item
    @ data_len int, data_len sizeof data
    @ num_items int, num_items sizeof lens

    lens : num_items <- len

    @ i int, i = 0
    while i < len
        @ char byte
        char <- item : i
        data : data_len <- char
        data_len += 1
        i += 1
    end
ef

fun Remove & data string & lens iarray @ pos int
    @ num_items int, num_items sizeof lens
    
    if pos >= num_items
        return
    end
    
    @ new_data string
    @ new_lens iarray
    
    @ i int, i = 0
    while i < num_items
        if i != pos
            @ item_len int
            item_len <- lens : i
            @ abs_pos int, abs_pos = 0
            
            @ k int, k = 0
            while k < i
                @ curr_len int
                curr_len <- lens : k
                abs_pos += curr_len
                k += 1
            end
            
            @ new_item string
            @ j int, j = 0
            while j < item_len
                @ char byte
                @ char_pos int, char_pos = abs_pos + j
                char <- data : char_pos
                new_item : j <- char
                j += 1
            end
            
            @ new_data_len int, new_data_len sizeof new_data
            @ new_lens_len int, new_lens_len sizeof new_lens
            
            new_lens : new_lens_len <- item_len
            
            j = 0
            while j < item_len
                @ char byte
                char <- new_item : j
                new_data : new_data_len <- char
                new_data_len += 1
                j += 1
            end
            
            free new_item
        end
        i += 1
    end
    
    data = new_data
    lens = new_lens
ef

# Usage example
fun Main
    @ data string
    @ lens iarray

    call Create data lens

    @ example string
    cat example const Apple
    call Add data lens example

    free example
    cat example const Pear
    call Add data lens example

    free example
    cat example const Monkey
    call Add data lens example
    
    call Remove data lens 1

    free example
    cat example const Banana
    call Add data lens example

    println string data
ef