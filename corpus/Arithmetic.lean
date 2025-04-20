prelude
import Init.Prelude

namespace Nat

protected def mod : Nat → Nat → Nat := sorry
protected def div : Nat → Nat → Nat := sorry

end Nat

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
