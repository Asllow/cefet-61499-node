function tick(in0)
  if init == nil then
    init = true
    ea = 0.0
    uia = 0.0
    ref = 0.96
    u0 = 4.8
    kp = 2.0858
    ki = 4.5005
    kd = 0.1038
    Ts = 0.054
  end
  local angle = in0 or 0.0
  local e = ref - angle
  local duia = ki * Ts * e
  local ud = (e - ea) * (kd / Ts)
  local u = (kp * e) + ud + uia + u0
  ea = e
  if u > 12.0 then u = 12.0 elseif u < 0.0 then u = 0.0 else uia = uia + duia end
  return (u / 12.0) * 100.0
end