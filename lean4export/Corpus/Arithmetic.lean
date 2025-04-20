prelude
import Corpus.ExtendedPrelude

theorem nat_add : Eq (Nat.add 1000001 1000001) 2000002 := rfl

theorem nat_sub₁ : Eq (Nat.sub 123456789101112131415 101112131415) 123456789000000000000 := rfl

theorem nat_sub₂ : Eq (Nat.sub 123456789 2000000000000) 0 := rfl

theorem nat_mul₁ : Eq (Nat.mul 1000000000000000 1000000000000000) 1000000000000000000000000000000 := rfl

theorem nat_mul₂ : Eq (Nat.mul 1000000000000000 1) 1000000000000000 := rfl

theorem nat_div₁ : Eq (Nat.div 1 0) 0 := rfl

theorem nat_div₂ : Eq (Nat.div 1000000000000000000000000000001 1000000000000000) 1000000000000000 := rfl

theorem nat_mod₁ : Eq (Nat.mod 1 0) 1 := rfl

theorem nat_mod₂ : Eq (Nat.mod 1000000000000000000000000000001 1000000000000000) 1 := rfl

theorem nat_mul₃ : Eq (Nat.mul 1000000000000000 0) 0 := rfl

theorem nat_pow : Eq (Nat.pow 10 15) 1000000000000000 := rfl

theorem nat_beq₁ : Eq (Nat.beq 1000000000000000000000000000000 1000000000000000000000000000001) false := rfl

theorem nat_beq₂ : Eq (Nat.beq 1000000000000000000000000000000 1000000000000000000000000000000) true := rfl

theorem nat_beq₃ : Eq (Nat.beq 1 0) false := rfl

theorem nat_ble₁ : Eq (Nat.ble 1000000000000000000000000000000 1000000000000000000000000000001) true := rfl

theorem nat_ble₂ : Eq (Nat.ble 1000000000000000000000000000002 1000000000000000000000000000001) false := rfl

theorem nat_ble₃ : Eq (Nat.ble 1 0) false := rfl

-- see type_checker.cpp:is_def_eq_zero
theorem nat_zero_eq_zero : Eq 0 0 := rfl

-- see type_checker.cpp:is_def_eq_succ
theorem  nat_succ_eq_succ (n : Nat) : Eq (Nat.succ n) (Nat.succ n) := rfl

theorem nat_land : Eq (Nat.land 3 5) 1 := rfl

theorem nat_lor : Eq (Nat.lor 3 5) 7 := rfl

theorem nat_shift_left : Eq (Nat.shiftLeft 1 10) 1024 := rfl

theorem nat_shift_right : Eq (Nat.shiftRight 1024 10) 1 := rfl
