-- data/script.lua
function tick(in_0)
    local setpoint = 50.0
    local erro = setpoint - in_0
    local kp = 2.5
    
    -- Zona Morta
    if erro > -10 and erro < 10 then 
        erro = 0 
    end
    
    local u = erro * kp
    
    -- Saturação
    if u > 1000 then 
        u = 1000 
    elseif u < -1000 then 
        u = -1000 
    end
    
    return erro, u
end